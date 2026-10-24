// Copyright Epic Games, Inc. All Rights Reserved.

#include "Bunny_Third_PersonCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Bunny_Third_Person.h"
#include "BP_ObjectGrab.h"
#include "CombatComponent.h"

ABunny_Third_PersonCharacter::ABunny_Third_PersonCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->SetGenerateOverlapEvents(true);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Character Movement setup
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	MoveComp->bOrientRotationToMovement = true;
	MoveComp->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	MoveComp->AirControl = 0.35f;
	MoveComp->GravityScale = DefaultGravityScale;
	MoveComp->JumpZVelocity = 500.f;
	MoveComp->MaxWalkSpeed = WalkSpeed;
	MoveComp->MinAnalogWalkSpeed = 20.f;
	MoveComp->BrakingDecelerationWalking = DefaultBrakingDecelWalking;
	MoveComp->BrakingDecelerationFalling = 1500.0f;

	MoveComp->GetNavAgentPropertiesRef().bCanCrouch = true;
	MoveComp->CrouchedHalfHeight = 48.0f;

	// Camera setup
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	HoldPoint = CreateDefaultSubobject<USceneComponent>(TEXT("HoldPoint"));
	HoldPoint->SetupAttachment(RootComponent);

	CombatComp = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
}

void ABunny_Third_PersonCharacter::BeginPlay()
{
	Super::BeginPlay();

	DefaultGravityScale = GetCharacterMovement()->GravityScale;
	DefaultBrakingDecelWalking = GetCharacterMovement()->BrakingDecelerationWalking;
	bCanAirDash = true;
}

void ABunny_Third_PersonCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 1. Ground Pound
	if (bIsGroundPounding)
	{
		GetCharacterMovement()->Velocity = FVector(0.0f, 0.0f, -GroundPoundSpeed);
		return;
	}

	// 2. Slope Slide
	TickSlopeSlide(DeltaTime);

	// 3. Moving Base Update
	if (bIsLedgeHanging || bIsWallSliding || bIsWallStick)
	{
		UpdateMovingBaseMovement();
	}
	else
	{
		MovingBaseComponent = nullptr;
	}

	// 4. Dash
	if (bIsDashing)
	{
		GetCharacterMovement()->Velocity = DashDirection * DashSpeed;
		return;
	}

	// 5. Ledge Hang & Shimmy
	if (bIsLedgeHanging)
	{
		GetCharacterMovement()->Velocity = FVector::ZeroVector;
		TickLedgeShimmy(DeltaTime);
		return;
	}

	// 6. Ledge Detection
	if (bCanLedgeGrab && GetCharacterMovement()->IsFalling() && GetCharacterMovement()->Velocity.Z <= 100.0f)
	{
		FVector FoundLedge, FoundWallNormal;
		UPrimitiveComponent* FoundComponent = nullptr;
		if (DetectLedge(FoundLedge, FoundWallNormal, FoundComponent))
		{
			StartLedgeGrab(FoundLedge, FoundWallNormal, FoundComponent);
			return;
		}
	}

	// 7. Wall Slide & Wall Detection
	if (bIsWallSliding)
	{
		FHitResult SlideHit;
		const FVector Start = GetActorLocation();
		const FVector End = Start - (WallNormal * 60.0f);

		FCollisionQueryParams TraceParams(FName(TEXT("WallSlideTrace")), true, this);
		TraceParams.AddIgnoredActor(this);

		const bool bHitWall = GetWorld()->LineTraceSingleByChannel(SlideHit, Start, End, ECC_WorldStatic, TraceParams);

		if (bHitWall && FMath::Abs(SlideHit.Normal.Z) < 0.35f)
		{
			WallNormal = SlideHit.Normal;
			const bool bPushIntoWall = IsPushingIntoWall();

			if (bPushIntoWall && !bHasWallStick)
			{
				StartWallStick();
			}
			else if (bIsWallStick && !bPushIntoWall)
			{
				OnWallStickEnd();
			}

			GetCharacterMovement()->Velocity = bIsWallStick ? FVector::ZeroVector : FVector(0.0f, 0.0f, -WallSlideSpeed);
		}
		else
		{
			StopWallSlide();
		}
	}
	else if (GetCharacterMovement()->IsFalling() && GetCharacterMovement()->Velocity.Z <= 50.0f)
	{
		FHitResult HitResult;
		const FVector Start = GetCapsuleComponent()->GetComponentLocation();
		const FVector End = Start + (GetActorForwardVector() * 55.0f);

		FCollisionQueryParams TraceParams(FName(TEXT("WallJumpTrace")), true, this);
		TraceParams.AddIgnoredActor(this);

		if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldStatic, TraceParams))
		{
			if (FMath::Abs(HitResult.Normal.Z) < 0.35f)
			{
				StartWallSlide(HitResult.Normal, HitResult.GetComponent());
			}
		}
	}
}

// -----------------------------------------------------------------------------
// Input Setup
// -----------------------------------------------------------------------------
void ABunny_Third_PersonCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInputComponent)
	{
		UE_LOG(LogBunny_Third_Person, Error, TEXT("'%s' Failed to find an Enhanced Input component!"), *GetNameSafe(this));
		return;
	}

	// Single binding for jump press; resolved in DoJumpStart()
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ABunny_Third_PersonCharacter::DoJumpStart);
	EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ABunny_Third_PersonCharacter::DoJumpEnd);

	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABunny_Third_PersonCharacter::Move);
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &ABunny_Third_PersonCharacter::MoveCompleted);
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Canceled, this, &ABunny_Third_PersonCharacter::MoveCompleted);

	EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABunny_Third_PersonCharacter::Look);
	EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ABunny_Third_PersonCharacter::Look);

	EnhancedInputComponent->BindAction(GrabAction, ETriggerEvent::Started, this, &ABunny_Third_PersonCharacter::DoGrab);

	EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &ABunny_Third_PersonCharacter::OnDashButtonPressed);
	EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Completed, this, &ABunny_Third_PersonCharacter::OnDashButtonReleased);
	EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Canceled, this, &ABunny_Third_PersonCharacter::OnDashButtonReleased);

	EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ABunny_Third_PersonCharacter::DoAttack);
	EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ABunny_Third_PersonCharacter::DoCrouch);
}

// -----------------------------------------------------------------------------
// Movement & Looking Callbacks
// -----------------------------------------------------------------------------
void ABunny_Third_PersonCharacter::Move(const FInputActionValue& Value)
{
	CurrentMoveInput = Value.Get<FVector2D>();

	if (bIsDashing || bIsGroundPounding) return;

	if (bIsLedgeHanging)
	{
		if (CurrentMoveInput.Y < -0.5f)
		{
			DropFromLedge();
		}
		return;
	}

	DoMove(CurrentMoveInput.X, CurrentMoveInput.Y);
}

void ABunny_Third_PersonCharacter::MoveCompleted(const FInputActionValue& Value)
{
	CurrentMoveInput = FVector2D::ZeroVector;
}

void ABunny_Third_PersonCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxis = Value.Get<FVector2D>();
	DoLook(LookAxis.X, LookAxis.Y);
}

void ABunny_Third_PersonCharacter::DoMove(float Right, float Forward)
{
	if (!Controller) return;

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, Forward);
	AddMovementInput(RightDirection, Right);
}

void ABunny_Third_PersonCharacter::DoLook(float Yaw, float Pitch)
{
	if (!Controller) return;
	AddControllerYawInput(Yaw);
	AddControllerPitchInput(Pitch);
}

// -----------------------------------------------------------------------------
// Unified Jump Logic
// -----------------------------------------------------------------------------
void ABunny_Third_PersonCharacter::DoJumpStart()
{
	if (bIsGroundPounding) return;

	if (bIsCrouched)
	{
		UnCrouch();
	}

	// 1. Ledge Climb / Drop
	if (bIsLedgeHanging)
	{
		if (CurrentMoveInput.Y < -0.2f)
		{
			DropFromLedge();
		}
		else
		{
			ClimbUpLedge();
		}
		return;
	}

	// 2. Wall Jump
	if (TryWallJump())
	{
		return;
	}

	// 3. Single Ground Jump
	if (CanJump())
	{
		Jump();
		return;
	}

	// 4. Object-Assisted Jump (Mid-air boost by throwing held object downwards)
	if (HeldGrabObject && GetCharacterMovement()->IsFalling())
	{
		const float CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		const FVector PlayerLocation = GetActorLocation();
		const float CapsuleBottomZ = PlayerLocation.Z - CapsuleHalfHeight;

		FVector Origin, BoxExtent;
		HeldGrabObject->GetActorBounds(true, Origin, BoxExtent);

		FHitResult GroundHit;
		const FVector TraceStart = PlayerLocation;
		const float TraceDistance = CapsuleHalfHeight + (BoxExtent.Z * 2.0f) + 100.0f;
		const FVector TraceEnd = TraceStart - FVector(0.0f, 0.0f, TraceDistance);

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);
		QueryParams.AddIgnoredActor(HeldGrabObject);

		FCollisionObjectQueryParams ObjectParams;
		ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
		ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);

		const bool bHitGround = GetWorld()->LineTraceSingleByObjectType(GroundHit, TraceStart, TraceEnd, ObjectParams, QueryParams);

		FVector FinalBoxLocation = FVector(PlayerLocation.X, PlayerLocation.Y, CapsuleBottomZ - BoxExtent.Z - 10.0f);
		FVector FinalImpulse = FVector(0.0f, 0.0f, -ObjectDownImpulse);

		if (bHitGround)
		{
			const float MinBoxZ = GroundHit.ImpactPoint.Z + BoxExtent.Z + 1.0f;
			if (FinalBoxLocation.Z < MinBoxZ)
			{
				FinalBoxLocation.Z = MinBoxZ;
				FinalImpulse = FVector::ZeroVector;
			}
		}

		HeldGrabObject->PlaceAndDrop(FinalBoxLocation, FinalImpulse);
		HeldGrabObject = nullptr;

		LaunchCharacter(FVector(0.0f, 0.0f, ObjectJumpBoost), false, true);
	}
}

void ABunny_Third_PersonCharacter::DoJumpEnd()
{
	StopJumping();
}

bool ABunny_Third_PersonCharacter::TryWallJump()
{
	if (!bCanWallJump && !bIsWallSliding)
	{
		return false;
	}

	StopWallSlide();

	const FVector JumpDirection = (WallNormal * WallJumpHorizontalForce) + FVector(0.0f, 0.0f, WallJumpVerticalForce);
	LaunchCharacter(JumpDirection, true, true);
	SetActorRotation(WallNormal.Rotation());

	bCanWallJump = false;
	return true;
}

// -----------------------------------------------------------------------------
// Object Grab
// -----------------------------------------------------------------------------
void ABunny_Third_PersonCharacter::DoGrab()
{
	if (HeldGrabObject)
	{
		HeldGrabObject->DropObject();
		HeldGrabObject = nullptr;
		return;
	}

	if (NearbyGrabObject && !NearbyGrabObject->IsHeld())
	{
		HeldGrabObject = NearbyGrabObject;
		HeldGrabObject->GrabObject(GetMesh(), FName("right_hand"));
	}
}

// -----------------------------------------------------------------------------
// Crouch & Ground Pound
// -----------------------------------------------------------------------------
void ABunny_Third_PersonCharacter::DoCrouch()
{
	if (bIsGroundPounding) return;

	const float CurrentTime = GetWorld()->GetTimeSeconds();

	if ((CurrentTime - LastCrouchPressTime) <= DoubleTapCrouchThreshold)
	{
		LastCrouchPressTime = -1.0f;
		StartGroundPound();
		return;
	}

	LastCrouchPressTime = CurrentTime;

	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void ABunny_Third_PersonCharacter::StartGroundPound()
{
	if (bIsGroundPounding) return;

	if (bIsLedgeHanging) DropFromLedge();
	StopWallSlide();
	if (bIsDashing) StopDash();
	if (bIsCrouched) UnCrouch();

	bIsGroundPounding = true;
	GetCharacterMovement()->SetMovementMode(MOVE_Falling);
	GetCharacterMovement()->Velocity = FVector(0.0f, 0.0f, -GroundPoundSpeed);
}

void ABunny_Third_PersonCharacter::StopGroundPound()
{
	if (!bIsGroundPounding) return;

	bIsGroundPounding = false;
	GetCharacterMovement()->GravityScale = DefaultGravityScale;
}

// -----------------------------------------------------------------------------
// Dash & Sprint
// -----------------------------------------------------------------------------
void ABunny_Third_PersonCharacter::OnDashButtonPressed()
{
	if (bIsDashing || bIsGroundPounding) return;

	GetWorldTimerManager().SetTimer(
		HoldToRunTimerHandle, this, &ABunny_Third_PersonCharacter::StartSprint, HoldToRunThreshold, false);
}

void ABunny_Third_PersonCharacter::OnDashButtonReleased()
{
	const bool bWasTap = GetWorldTimerManager().IsTimerActive(HoldToRunTimerHandle);
	GetWorldTimerManager().ClearTimer(HoldToRunTimerHandle);

	if (bIsSprinting)
	{
		StopSprint();
	}
	else if (bWasTap)
	{
		StartDash();
	}
}

void ABunny_Third_PersonCharacter::StartSprint()
{
	if (!GetCharacterMovement()->IsMovingOnGround()) return;

	bIsSprinting = true;
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void ABunny_Third_PersonCharacter::StopSprint()
{
	bIsSprinting = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void ABunny_Third_PersonCharacter::StartDash()
{
	if (bIsDashing || bDashOnCoolDown || bIsGroundPounding) return;

	if (bIsLedgeHanging) DropFromLedge();
	StopWallSlide();

	const bool bIsGrounded = GetCharacterMovement()->IsMovingOnGround();
	bool bIsObjectAirDash = false;

	if (!bIsGrounded)
	{
		if (bCanAirDash)
		{
			bCanAirDash = false;
		}
		else if (HeldGrabObject)
		{
			bIsObjectAirDash = true;
		}
		else
		{
			return;
		}
	}

	if (!CurrentMoveInput.IsNearlyZero())
	{
		const FRotator ControlRot = Controller ? Controller->GetControlRotation() : GetActorRotation();
		const FRotator YawRot(0.f, ControlRot.Yaw, 0.f);

		const FVector CameraForward = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
		const FVector CameraRight = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

		DashDirection = (CameraForward * CurrentMoveInput.Y) + (CameraRight * CurrentMoveInput.X);
	}
	else
	{
		DashDirection = GetActorForwardVector();
	}

	DashDirection.Z = 0.0f;
	DashDirection.Normalize();

	FRotator FaceRotate = DashDirection.Rotation();
	FaceRotate.Pitch = 0.0f;
	FaceRotate.Roll = 0.0f;
	SetActorRotation(FaceRotate);

	// Object ejection during air dash
	if (bIsObjectAirDash && HeldGrabObject)
	{
		FVector Origin, BoxExtent;
		HeldGrabObject->GetActorBounds(true, Origin, BoxExtent);

		const float OffsetDist = GetCapsuleComponent()->GetScaledCapsuleRadius() + FMath::Max(BoxExtent.X, BoxExtent.Y) + 20.0f;
		const FVector TraceStart = GetActorLocation();
		const FVector TraceEnd = TraceStart - (DashDirection * OffsetDist);

		FHitResult WallHit;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);
		QueryParams.AddIgnoredActor(HeldGrabObject);

		FVector DropLocation = TraceEnd;
		if (GetWorld()->LineTraceSingleByChannel(WallHit, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
		{
			DropLocation = WallHit.Location + (DashDirection * 10.0f);
		}

		HeldGrabObject->PlaceAndDrop(DropLocation, -DashDirection * ObjectDownImpulse);
		HeldGrabObject = nullptr;
	}

	bIsDashing = true;
	bDashOnCoolDown = true;

	GetCharacterMovement()->GravityScale = 0.0f;
	GetCharacterMovement()->Velocity = DashDirection * DashSpeed;

	GetWorldTimerManager().SetTimer(DashTimeHandle, this, &ABunny_Third_PersonCharacter::StopDash, DashDuration, false);
	GetWorldTimerManager().SetTimer(DashCooldownTimerHandle, this, &ABunny_Third_PersonCharacter::ResetDashCooldown, DashDuration + DashCooldown, false);
}

void ABunny_Third_PersonCharacter::StopDash()
{
	if (!bIsDashing) return;

	bIsDashing = false;
	GetCharacterMovement()->GravityScale = DefaultGravityScale;
	GetCharacterMovement()->Velocity = GetCharacterMovement()->Velocity.GetClampedToMaxSize(DashExitSpeed);
}

void ABunny_Third_PersonCharacter::ResetDashCooldown()
{
	bDashOnCoolDown = false;
}

void ABunny_Third_PersonCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (bIsGroundPounding) StopGroundPound();
	if (bIsLedgeHanging)   DropFromLedge();

	StopWallSlide();

	bCanLedgeGrab = true;
	bCanAirDash = true;
	GetWorldTimerManager().ClearTimer(LedgeCooldownTimerHandle);
}

// -----------------------------------------------------------------------------
// Wall Slide & Wall Stick
// -----------------------------------------------------------------------------
void ABunny_Third_PersonCharacter::StartWallSlide(const FVector& HitNormal, UPrimitiveComponent* HitComponent)
{
	if (bIsGroundPounding) return;

	bIsWallSliding = true;
	bCanWallJump = true;
	bIsWallStick = false;
	bHasWallStick = false;
	WallNormal = HitNormal;

	if (HitComponent && HitComponent->Mobility == EComponentMobility::Movable)
	{
		MovingBaseComponent = HitComponent;
		PreviousBaseTransform = HitComponent->GetComponentTransform();
	}
	else
	{
		MovingBaseComponent = nullptr;
	}

	FRotator FaceWallRotation = (-WallNormal).Rotation();
	FaceWallRotation.Pitch = 0.0f;
	FaceWallRotation.Roll = 0.0f;
	SetActorRotation(FaceWallRotation);

	GetCharacterMovement()->GravityScale = 0.0f;

	if (IsPushingIntoWall())
	{
		StartWallStick();
	}
	else
	{
		GetCharacterMovement()->Velocity = FVector(0.0f, 0.0f, -WallSlideSpeed);
	}
}

void ABunny_Third_PersonCharacter::StartWallStick()
{
	bIsWallStick = true;
	bHasWallStick = true;
	GetCharacterMovement()->Velocity = FVector::ZeroVector;

	GetWorldTimerManager().ClearTimer(WallStickTimeHandle);
	GetWorldTimerManager().SetTimer(WallStickTimeHandle, this, &ABunny_Third_PersonCharacter::OnWallStickEnd, WallStickDuration, false);
}

void ABunny_Third_PersonCharacter::OnWallStickEnd()
{
	bIsWallStick = false;
	GetWorldTimerManager().ClearTimer(WallStickTimeHandle);
}

void ABunny_Third_PersonCharacter::StopWallSlide()
{
	if (!bIsWallSliding && !bCanWallJump) return;

	bIsWallSliding = false;
	bIsWallStick = false;
	bCanWallJump = false;
	bHasWallStick = false;

	GetWorldTimerManager().ClearTimer(WallStickTimeHandle);
	GetCharacterMovement()->GravityScale = DefaultGravityScale;
}

bool ABunny_Third_PersonCharacter::IsPushingIntoWall() const
{
	if (CurrentMoveInput.IsNearlyZero() || !Controller) return false;

	const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	const FVector InputWorldDir = (ForwardDirection * CurrentMoveInput.Y + RightDirection * CurrentMoveInput.X).GetSafeNormal();

	return FVector::DotProduct(InputWorldDir, WallNormal) < -0.4f;
}

// -----------------------------------------------------------------------------
// Ledge Mechanics & Shared Trace
// -----------------------------------------------------------------------------
bool ABunny_Third_PersonCharacter::TraceLedgeSurface(
	const FVector& RayStart,
	const FVector& ForwardDir,
	float ForwardDist,
	FHitResult& OutWallHit,
	FHitResult& OutLedgeHit) const
{
	FCollisionQueryParams TraceParams(FName(TEXT("LedgeSurfaceTrace")), true, this);
	TraceParams.AddIgnoredActor(this);
	if (HeldGrabObject)
	{
		TraceParams.AddIgnoredActor(HeldGrabObject);
	}

	// 1. Raycast forward to find wall surface
	const FVector ForwardEnd = RayStart + (ForwardDir * ForwardDist);
	if (!GetWorld()->LineTraceSingleByChannel(OutWallHit, RayStart, ForwardEnd, ECC_WorldStatic, TraceParams))
	{
		return false;
	}

	if (FMath::Abs(OutWallHit.Normal.Z) > 0.15f)
	{
		return false;
	}

	// 2. Raycast downward into wall to find top flat ledge edge
	const float CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	FVector DownStart = OutWallHit.ImpactPoint - (OutWallHit.Normal * 15.0f);
	DownStart.Z = GetActorLocation().Z + CapsuleHalfHeight + LedgeGrabTraceHeight;
	const FVector DownEnd = FVector(DownStart.X, DownStart.Y, GetActorLocation().Z - 30.0f);

	if (!GetWorld()->LineTraceSingleByChannel(OutLedgeHit, DownStart, DownEnd, ECC_WorldStatic, TraceParams))
	{
		return false;
	}

	return (!OutLedgeHit.bStartPenetrating && OutLedgeHit.Normal.Z >= 0.7f);
}

bool ABunny_Third_PersonCharacter::DetectLedge(FVector& OutLedgeLoc, FVector& OutWallNormal, UPrimitiveComponent*& OutHitComponent)
{
	if (bIsGroundPounding) return false;

	OutHitComponent = nullptr;

	FHitResult WallHit, LedgeHit;
	const FVector ForwardStart = GetActorLocation() + FVector(0.0f, 0.0f, 30.0f);

	if (!TraceLedgeSurface(ForwardStart, GetActorForwardVector(), LedgeGrabForwardReach, WallHit, LedgeHit))
	{
		return false;
	}

	OutLedgeLoc = LedgeHit.ImpactPoint;
	OutWallNormal = WallHit.Normal;
	OutHitComponent = WallHit.GetComponent() ? WallHit.GetComponent() : LedgeHit.GetComponent();
	return true;
}

void ABunny_Third_PersonCharacter::StartLedgeGrab(const FVector& InLedgeLoc, const FVector& InWallNormal, UPrimitiveComponent* HitComponent)
{
	if (bIsGroundPounding) return;

	bIsLedgeHanging = true;
	LedgeLocation = InLedgeLoc;
	LedgeWallNormal = InWallNormal;

	if (HitComponent && HitComponent->Mobility == EComponentMobility::Movable)
	{
		MovingBaseComponent = HitComponent;
		PreviousBaseTransform = HitComponent->GetComponentTransform();
	}
	else
	{
		MovingBaseComponent = nullptr;
	}

	StopWallSlide();

	GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	GetCharacterMovement()->Velocity = FVector::ZeroVector;
	GetCharacterMovement()->GravityScale = 0.0f;

	const float CapsuleRadius = GetCapsuleComponent()->GetScaledCapsuleRadius();
	FVector SnapLocation = LedgeLocation + (LedgeWallNormal * (CapsuleRadius + 10.0f));
	SnapLocation.Z = LedgeLocation.Z - LedgeHangVerticalOffset;
	SetActorLocation(SnapLocation);

	FRotator FaceWall = (-LedgeWallNormal).Rotation();
	FaceWall.Pitch = 0.0f;
	FaceWall.Roll = 0.0f;
	SetActorRotation(FaceWall);
}

void ABunny_Third_PersonCharacter::DropFromLedge()
{
	if (!bIsLedgeHanging) return;

	bIsLedgeHanging = false;
	bCanLedgeGrab = false;

	GetCharacterMovement()->SetMovementMode(MOVE_Falling);
	GetCharacterMovement()->GravityScale = DefaultGravityScale;
	GetCharacterMovement()->Velocity = (LedgeWallNormal * 150.0f) + FVector(0.0f, 0.0f, -100.0f);

	GetWorldTimerManager().SetTimer(
		LedgeCooldownTimerHandle, this, &ABunny_Third_PersonCharacter::ResetLedgeGrabCooldown, LedgeGrabCooldown, false);
}

void ABunny_Third_PersonCharacter::ClimbUpLedge()
{
	if (!bIsLedgeHanging) return;

	bIsLedgeHanging = false;
	bCanLedgeGrab = true;
	GetWorldTimerManager().ClearTimer(LedgeCooldownTimerHandle);

	const float CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const float CapsuleRadius = GetCapsuleComponent()->GetScaledCapsuleRadius();

	FVector TargetLocation = GetActorLocation() - (LedgeWallNormal * (CapsuleRadius + 20.0f));
	TargetLocation.Z = LedgeLocation.Z + CapsuleHalfHeight + 5.0f;

	SetActorLocation(TargetLocation, false, nullptr, ETeleportType::TeleportPhysics);

	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	GetCharacterMovement()->GravityScale = DefaultGravityScale;
	GetCharacterMovement()->Velocity = FVector::ZeroVector;
}

void ABunny_Third_PersonCharacter::ResetLedgeGrabCooldown()
{
	bCanLedgeGrab = true;
}

bool ABunny_Third_PersonCharacter::CanShimmy(float DirectionSign, FVector& OutLedgeLoc, FVector& OutWallNormal)
{
	const FVector WallRight = FVector::CrossProduct(LedgeWallNormal, FVector::UpVector).GetSafeNormal();
	const float CapsuleRadius = GetCapsuleComponent()->GetScaledCapsuleRadius();

	const FVector SideOffset = WallRight * DirectionSign * 20.0f;
	const FVector ForwardStart = GetActorLocation() + SideOffset + FVector(0.0f, 0.0f, 30.0f);

	FHitResult WallHit, LedgeHit;
	if (!TraceLedgeSurface(ForwardStart, -LedgeWallNormal, CapsuleRadius + 40.0f, WallHit, LedgeHit))
	{
		return false;
	}

	OutLedgeLoc = FVector(WallHit.ImpactPoint.X, WallHit.ImpactPoint.Y, LedgeHit.ImpactPoint.Z);
	OutWallNormal = WallHit.Normal;
	return true;
}

void ABunny_Third_PersonCharacter::TickLedgeShimmy(float DeltaTime)
{
	if (FMath::Abs(CurrentMoveInput.X) < 0.15f) return;

	const float DirectionSign = FMath::Sign(CurrentMoveInput.X);
	FVector AheadLedgeLoc, NewWallNormal;
	if (!CanShimmy(DirectionSign, AheadLedgeLoc, NewWallNormal))
	{
		return;
	}

	const FVector WallRight = FVector::CrossProduct(LedgeWallNormal, FVector::UpVector).GetSafeNormal();
	const FVector DesiredMovement = WallRight * (CurrentMoveInput.X * LedgeShimmySpeed * DeltaTime);
	AddActorWorldOffset(DesiredMovement, false);

	const float CapsuleRadius = GetCapsuleComponent()->GetScaledCapsuleRadius();
	const FVector ForwardStart = GetActorLocation() + FVector(0.0f, 0.0f, 30.0f);

	FHitResult WallHit, LedgeHit;
	if (TraceLedgeSurface(ForwardStart, -LedgeWallNormal, CapsuleRadius + 40.0f, WallHit, LedgeHit))
	{
		LedgeWallNormal = WallHit.Normal;
		LedgeLocation = FVector(WallHit.ImpactPoint.X, WallHit.ImpactPoint.Y, LedgeHit.ImpactPoint.Z);

		FVector SnappedLocation = WallHit.ImpactPoint + (LedgeWallNormal * (CapsuleRadius + 5.0f));
		SnappedLocation.Z = LedgeLocation.Z - LedgeHangVerticalOffset;
		SetActorLocation(SnappedLocation, false);
	}
	else
	{
		FVector TargetPos = GetActorLocation();
		TargetPos.Z = LedgeLocation.Z - LedgeHangVerticalOffset;
		SetActorLocation(TargetPos, false);
	}

	FRotator FaceWall = (-LedgeWallNormal).Rotation();
	FaceWall.Pitch = 0.0f;
	FaceWall.Roll = 0.0f;
	SetActorRotation(FaceWall);
}

void ABunny_Third_PersonCharacter::UpdateMovingBaseMovement()
{
	if (!MovingBaseComponent.IsValid()) return;

	const FTransform CurrentBaseTransform = MovingBaseComponent->GetComponentTransform();
	const FQuat DeltaRotation = CurrentBaseTransform.GetRotation() * PreviousBaseTransform.GetRotation().Inverse();
	const FTransform LocalTransform = GetActorTransform().GetRelativeTransform(PreviousBaseTransform);
	const FTransform NewWorldTransform = LocalTransform * CurrentBaseTransform;

	SetActorLocationAndRotation(
		NewWorldTransform.GetLocation(),
		NewWorldTransform.GetRotation(),
		false,
		nullptr,
		ETeleportType::TeleportPhysics);

	LedgeWallNormal = DeltaRotation.RotateVector(LedgeWallNormal);
	WallNormal = DeltaRotation.RotateVector(WallNormal);
	LedgeLocation = CurrentBaseTransform.TransformPosition(PreviousBaseTransform.InverseTransformPosition(LedgeLocation));

	PreviousBaseTransform = CurrentBaseTransform;
}

// -----------------------------------------------------------------------------
// Slope Slide
// -----------------------------------------------------------------------------
void ABunny_Third_PersonCharacter::TickSlopeSlide(float DeltaTime)
{
	if (!bIsCrouched || !GetCharacterMovement()->IsMovingOnGround())
	{
		if (bIsSlopeSliding)
		{
			bIsSlopeSliding = false;
			GetCharacterMovement()->BrakingDecelerationWalking = DefaultBrakingDecelWalking;
		}
		return;
	}

	const FFindFloorResult& CurrentFloor = GetCharacterMovement()->CurrentFloor;
	if (!CurrentFloor.IsWalkableFloor())
	{
		bIsSlopeSliding = false;
		return;
	}

	const FVector FloorNormal = CurrentFloor.HitResult.ImpactNormal;
	const float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FloorNormal.Z));

	if (SlopeAngle >= MinSlopeSlideAngle)
	{
		bIsSlopeSliding = true;

		const FVector DownVector(0.0f, 0.0f, -1.0f);
		const FVector SlideDirection = FVector::VectorPlaneProject(DownVector, FloorNormal).GetSafeNormal();

		GetCharacterMovement()->BrakingDecelerationWalking = 0.0f;

		FVector CurrentVelocity = GetCharacterMovement()->Velocity;
		CurrentVelocity += SlideDirection * (SlopeSlideAcceleration * DeltaTime);
		CurrentVelocity = CurrentVelocity.GetClampedToMaxSize(MaxSlopeSlideSpeed);
		GetCharacterMovement()->Velocity = CurrentVelocity;

		FRotator TargetRotation = SlideDirection.Rotation();
		TargetRotation.Pitch = 0.0f;
		TargetRotation.Roll = 0.0f;
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, 12.0f));
	}
	else if (bIsSlopeSliding)
	{
		bIsSlopeSliding = false;
		GetCharacterMovement()->BrakingDecelerationWalking = DefaultBrakingDecelWalking;
	}
}

void ABunny_Third_PersonCharacter::DoAttack()
{
	if (CombatComp)
	{
		CombatComp->Attack();
	}
}