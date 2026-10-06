// Copyright Epic Games, Inc. All Rights Reserved.

#include "Bunny_Third_PersonCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Bunny_Third_Person.h"
#include "Kismet/KismetMathLibrary.h"
#include <BP_ObjectGrab.h>
#include "CombatComponent.h"
#include "Audio/AudioTraceUtil.h"

ABunny_Third_PersonCharacter::ABunny_Third_PersonCharacter()
{
	//Time
	PrimaryActorTick.bCanEverTick = true;

	// collision capsule Settigns
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->SetGenerateOverlapEvents(true);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	GetCharacterMovement()->AirControl = 0.35f;

	// testing gravity
	DefaultGravityScale = 1.75f;
	GetCharacterMovement()->GravityScale = DefaultGravityScale;

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Create a grabable values
	HoldPoint = CreateDefaultSubobject<USceneComponent>(TEXT("HoldPoint"));
	HoldPoint->SetupAttachment(RootComponent);

	HeldGrabObject = nullptr;
	NearbyGrabObject = nullptr;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	// Nathan - Creating Combat Component
	CombatComp = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
}

void ABunny_Third_PersonCharacter::BeginPlay()
{
	Super::BeginPlay();

	DefaultGravityScale = GetCharacterMovement()->GravityScale;
	WalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
	bCanAirDash = true;
}

void ABunny_Third_PersonCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	bJustWallJumped = false;

	// Moving wall logic
	if (bIsLedgeHanging || bIsWallSliding || bIsWallStick)
	{
		UpdateMovingBaseMovement();
	}
	else
	{
		MovingBaseComponent = nullptr;
	}

	// Dash Logic
	if (bIsDashing)
	{
		GetCharacterMovement()->Velocity = DashDirection * DashSpeed;
		return;
	}

	if (bIsLedgeHanging)
	{
		GetCharacterMovement()->Velocity = FVector::ZeroVector;
		TickLedgeShimmy(DeltaTime);
		return;
	}

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

	//Movement Wall Logic
	if (bIsWallSliding)
	{
		// Trace if contact is maintained 
		FHitResult SlideHit;
		FVector Start = GetActorLocation();
		FVector End = Start - (WallNormal * 60.0f);

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
			if (bIsWallStick && !bPushIntoWall)
			{
				OnWallStickEnd();
			}

			// Stick to the wall for sec
			if (bIsWallStick)
			{
				GetCharacterMovement()->Velocity = FVector::ZeroVector;
			}
			else
			{
				GetCharacterMovement()->Velocity = FVector(0.0f, 0.0f, -WallSlideSpeed);
			}
		}
		else
		{
			StopWallSlide();
		}
	}
	else if (GetCharacterMovement()->IsFalling())
	{
		// looking for wall when falling
		if (GetCharacterMovement()-> Velocity.Z <= 50.0f)
		{
			FHitResult HitResult;
			FVector Start = GetCapsuleComponent()->GetComponentLocation();
			FVector End = Start + (GetActorForwardVector() * 55.0f);

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
}

void ABunny_Third_PersonCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ABunny_Third_PersonCharacter::DoWallJumpLedge);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ABunny_Third_PersonCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ABunny_Third_PersonCharacter::DoJumpEnd);
		

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABunny_Third_PersonCharacter::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &ABunny_Third_PersonCharacter::MoveCompleted);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Canceled, this, &ABunny_Third_PersonCharacter::MoveCompleted);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ABunny_Third_PersonCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABunny_Third_PersonCharacter::Look);

		// Grab
		EnhancedInputComponent->BindAction(GrabAction, ETriggerEvent::Started, this, & ABunny_Third_PersonCharacter::DoGrab);

		// Dash
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &ABunny_Third_PersonCharacter::OnDashButtonPressed);
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Completed, this, &ABunny_Third_PersonCharacter::OnDashButtonReleased);
		EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Canceled, this, &ABunny_Third_PersonCharacter::OnDashButtonReleased);

		// Attacking
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ABunny_Third_PersonCharacter::DoAttack);

		// Crouch
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ABunny_Third_PersonCharacter::DoCrouch);
	}
	else
	{
		UE_LOG(LogBunny_Third_Person, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ABunny_Third_PersonCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	CurrentMoveInput = Value.Get<FVector2D>();

	if (bIsDashing) return;
	
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
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void ABunny_Third_PersonCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void ABunny_Third_PersonCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void ABunny_Third_PersonCharacter::DoJumpStart()
{
	// If Holding on ladge
	if (bIsLedgeHanging)
	{
		// if moved back, drop down
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
	
	
	// No Object movement on a wall 
	if (bJustWallJumped || bIsWallSliding)
	{
		return;
	}

	// signal the character to jump
	if (CanJump())
	{
		Jump();
		return;
	}

	// if player have normal jump and dubble 
	if (HeldGrabObject && GetCharacterMovement()->IsFalling())
	{
		/**
		Add back when test done
		*/
		//// Object under his feet
		//float HalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		//FVector DropLocation = GetActorLocation() - FVector(0.0f, 0.0f, HalfHeight + 35.0f);
		//HeldGrabObject->SetActorLocation(DropLocation);

		/**
		Test idea for object underplayer
		*/
		const float CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		const FVector PlayerLocation = GetActorLocation();
		const float CapsuleBottomZ = PlayerLocation.Z - CapsuleHalfHeight; //Get's the Capsule Z location 

		FVector Origin, BoxExtent; 
		HeldGrabObject->GetActorBounds(true, Origin, BoxExtent);

		// -- RAYCAST for ground
		FHitResult GroundHit;
		FVector TraceStart = PlayerLocation;

		float TraceDistance = CapsuleHalfHeight + (BoxExtent.Z * 2.0f) + 100.0f;
		FVector TraceEnd = TraceStart - FVector(0.0f, 0.0f, TraceDistance);

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);
		QueryParams.AddIgnoredActor(HeldGrabObject);

		FCollisionObjectQueryParams ObjectParams; 
		ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
		ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);

		bool bHitGround = GetWorld()->LineTraceSingleByObjectType(
			GroundHit,
			TraceStart,
			TraceEnd,
			ObjectParams,
			QueryParams
		);

		// Mid-air placement
		FVector FinelBoxLocation = FVector(PlayerLocation.X, PlayerLocation.Y, CapsuleBottomZ - BoxExtent.Z - 10.0f);
		FVector FinelImpulse = FVector(0.0f, 0.0f, -ObjectDownImpulse); // hold

		//Place Objcet on ground whem raycast is close to the ground 
		if (bHitGround)
		{
			const float GroundZ = GroundHit.ImpactPoint.Z;
			const float MinBoxZ = GroundZ + BoxExtent.Z + 1.0f;

			if (FinelBoxLocation.Z < MinBoxZ)
			{
				FinelBoxLocation.Z = MinBoxZ;
				FinelImpulse = FVector::ZeroVector;
			}
		}
		
		HeldGrabObject->PlaceAndDrop(FinelBoxLocation, FinelImpulse);
		HeldGrabObject = nullptr;
		/**
		Test code end
		*/

		// The jump
		LaunchCharacter(FVector(0.0f, 0.0f, ObjectJumpBoost), false, true);
	}
}

void ABunny_Third_PersonCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

void ABunny_Third_PersonCharacter::DoGrab()
{
	// if holding a object, drop it
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

void ABunny_Third_PersonCharacter::DoCrouch()
{
}

void ABunny_Third_PersonCharacter::StartDash()
{
	if (bIsDashing || bDashOnCoolDown) return;

	if (bIsLedgeHanging)
	{
		DropFromLedge();
	}

	StopWallSlide();

	const bool bIsGrounded = GetCharacterMovement()->IsMovingOnGround();
	bool bIsObjectAirDash = false;

	// Dash Logic 
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

	// cheack if player press (WASD)
	if (!CurrentMoveInput.IsNearlyZero()) 
	{
		FRotator ControlRot = Controller ? Controller->GetControlRotation() : GetActorRotation();
		FRotator YawRot(0.f, ControlRot.Yaw, 0.f);

		const FVector CameraForward = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
		const FVector CameraRight = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

		DashDirection = (CameraForward * CurrentMoveInput.Y) + (CameraRight * CurrentMoveInput.X);
	}
	else { // Dash where Model is looking 
		DashDirection = GetActorForwardVector();
	}
	
	DashDirection.Z = 0.0f;
	DashDirection.Normalize();

	//Rotate the player to the dash
	FRotator FaceRotate = DashDirection.Rotation();
	FaceRotate.Pitch = 0.0f;
	FaceRotate.Roll = 0.0f;
	SetActorRotation(FaceRotate);

	// Air Dash Logic	
	if (bIsObjectAirDash && HeldGrabObject)
	{
		FVector Origin, BoxExtent;
		HeldGrabObject->GetActorBounds(true, Origin, BoxExtent);

		// Send Object Behind Player
		const float OffsetDistance = GetCapsuleComponent()->GetScaledCapsuleRadius() + FMath::Max(BoxExtent.X, BoxExtent.Y) + 20.0f;
		const FVector TraceStart = GetActorLocation();
		const FVector TraceEnd = TraceStart - (DashDirection * OffsetDistance);

		// Prevent clipping 
		FHitResult WallHit;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);
		QueryParams.AddIgnoredActor(HeldGrabObject);

		FVector DropLocation = TraceEnd;
		if (GetWorld()->LineTraceSingleByChannel(WallHit, TraceStart, TraceEnd, ECollisionChannel::ECC_WorldStatic, QueryParams))
		{
			DropLocation = WallHit.Location + (DashDirection * 10.0f);
		}

		//Push Object
		const FVector PushImpulse = -DashDirection * ObjectDownImpulse;
		HeldGrabObject->PlaceAndDrop(DropLocation, PushImpulse);
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
	FVector CurrentVel = GetCharacterMovement()->Velocity;
	GetCharacterMovement()->Velocity = CurrentVel.GetClampedToMaxSize(DashExitSpeed);
}

void ABunny_Third_PersonCharacter::ResetDashCooldown()
{
	bDashOnCoolDown = false;
}

void ABunny_Third_PersonCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	bCanLedgeGrab = true;
	GetWorldTimerManager().ClearTimer(LedgeCooldownTimerHandle);

	if (bIsLedgeHanging)
	{
		DropFromLedge();
	}

	StopWallSlide();
	bCanAirDash = true;
}

void ABunny_Third_PersonCharacter::DoWallJumpLedge()
{
	if (bCanWallJump || bIsWallSliding)
	{
		StopWallSlide();

		// Calculate Jump direction
		const FVector JumpDirection = (WallNormal * WallJumpHorizontalForce) + FVector(0.0f, 0.0f, WallJumpVerticalForce);
		LaunchCharacter(JumpDirection, true, true);

		GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Black, TEXT("Wall Jump"));

		// Rotate character away from wall ??? change for the animation 
		const FRotator NewRotation = WallNormal.Rotation();
		SetActorRotation(NewRotation);

		bCanWallJump = false;
	}
}

void ABunny_Third_PersonCharacter::StartWallSlide(const FVector& HitNormal, UPrimitiveComponent* HitComponent)
{
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
	GetWorldTimerManager().SetTimer(WallStickTimeHandle, 
		this, &ABunny_Third_PersonCharacter::OnWallStickEnd, 
		WallStickDuration, false);
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
	if (CurrentMoveInput.IsNearlyZero() || GetController() == nullptr)
	{
		return false;
	}
	
	// Calculate the World space movement input direction 
	const FRotator YawRotation(0.0f, GetController()->GetControlRotation().Yaw, 0.0f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	const FVector InputWorldDir = (ForwardDirection * CurrentMoveInput.Y + RightDirection * CurrentMoveInput.X).GetSafeNormal();

	return FVector::DotProduct(InputWorldDir, WallNormal) < -0.4f;
}

void ABunny_Third_PersonCharacter::OnDashButtonPressed()
{
	if (bIsDashing) return;

	//Set Timer
	GetWorldTimerManager().SetTimer(
		HoldToRunTimerHandle,
		this,
		&ABunny_Third_PersonCharacter::StartSprint,
		HoldToRunThresHold,
		false
	);

}

void ABunny_Third_PersonCharacter::OnDashButtonReleased()
{
	//Reset Timer
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

// Nathan - Attack Function
void ABunny_Third_PersonCharacter::DoAttack()
{

	if (CombatComp) {

		CombatComp->Attack();
	}
}

// Trace to find the ledge
bool ABunny_Third_PersonCharacter::DetectLedge(FVector& OutLedgeLoc, FVector& OutWallNormal, UPrimitiveComponent*& OutHitComponent)
{
	OutHitComponent = nullptr;
	
	FCollisionQueryParams TraceParams(FName(TEXT("LedgeTrace")), true, this );
	TraceParams.AddIgnoredActor(this);
	if (HeldGrabObject)
	{
		TraceParams.AddIgnoredActor(HeldGrabObject);
	}

	const FVector ActorLoc = GetActorLocation();
	const FVector Forward = GetActorForwardVector();

	// Place it in head / eye lvl
	FVector ForwardStart = ActorLoc + FVector(0.0f, 0.0f, 30.0f);
	FVector ForwardEnd = ForwardStart + (Forward * LedgeGrabForwardReach);

	FHitResult WallHit; 
	bool bHitWall = GetWorld()->LineTraceSingleByChannel(
		WallHit, ForwardStart, ForwardEnd, ECC_WorldStatic, TraceParams);

	if (!bHitWall || FMath::Abs(WallHit.Normal.Z) > 0.15f)
	{
		return false;
	}

	// to find the top surface of the wall
	const float CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	FVector DownStart = WallHit.ImpactPoint - (WallHit.Normal * 15.0f);
	DownStart.Z = ActorLoc.Z + CapsuleHalfHeight + LedgeGrabTraceHeight;
	FVector DownEnd = FVector(DownStart.X, DownStart.Y, ActorLoc.Z);

	FHitResult LedgeHit;
	bool bHitLedge = GetWorld()->LineTraceSingleByChannel(
		LedgeHit, DownStart, DownEnd, ECC_WorldStatic, TraceParams);

	if (!bHitLedge || LedgeHit.bStartPenetrating)
	{
		return false;
	}

	// flat scale
	if (LedgeHit.Normal.Z < 0.7f)
	{
		return false;
	}

	OutLedgeLoc = LedgeHit.ImpactPoint;
	OutWallNormal = WallHit.Normal;

	OutHitComponent = WallHit.GetComponent() ? WallHit.GetComponent() : LedgeHit.GetComponent();
	return true;
}

void ABunny_Third_PersonCharacter::StartLedgeGrab(const FVector& InLedgeLoc, const FVector& InWallNormal,
	UPrimitiveComponent* HitComponent)
{
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

	// Freeze Player
	GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	GetCharacterMovement()->Velocity = FVector::ZeroVector;
	GetCharacterMovement()->GravityScale = 0.0f;

	// Positon character up
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

	const FVector PushAwayForce = (LedgeWallNormal * 150.0f) + FVector(0.0f, 0.0f, -100.0f);
	GetCharacterMovement()->Velocity = PushAwayForce;

	GetWorldTimerManager().SetTimer(
		LedgeCooldownTimerHandle, this,
		&ABunny_Third_PersonCharacter::ResetLedgeGrabCooldown,
		LedgeGrabCooldown, false);
}

void ABunny_Third_PersonCharacter::ClimbUpLedge()
{
	if (!bIsLedgeHanging) return;

	bIsLedgeHanging = false;
	bCanLedgeGrab = true;
	GetWorldTimerManager().ClearTimer(LedgeCooldownTimerHandle);

	//Target top of the ledge 
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
	// Find the vector pointing to the player along the wall 
	const FVector WallRight = FVector::CrossProduct(LedgeWallNormal, FVector::UpVector).GetSafeNormal();
	const float CapsuleRadius = GetCapsuleComponent()->GetScaledCapsuleRadius();

	const float CheckDistance = 20.0f;
	const FVector SideOffset = WallRight * DirectionSign * CheckDistance;
	const FVector TestOrigin = GetActorLocation() + SideOffset;

	FCollisionQueryParams TraceParams(FName(TEXT("LedgeMoveTrace")), true, this);
	TraceParams.AddIgnoredActor(this);
	if (HeldGrabObject)
	{
		TraceParams.AddIgnoredActor(HeldGrabObject);
	}

	//find the wall
	const FVector ForwardStart = TestOrigin + FVector(0.0f, 0.0f, 30.0f);
	const FVector ForwardEnd = ForwardStart + (-LedgeWallNormal * (CapsuleRadius + 40.0f));

	FHitResult WallHit;
	bool bHitWall = GetWorld()->LineTraceSingleByChannel(WallHit, ForwardStart, ForwardEnd, ECC_WorldStatic, TraceParams);

	if (!bHitWall || FMath::Abs(WallHit.Normal.Z) > 0.15f)
	{
		return false;
	}

	// Trace downward so it can comfirm top ledge
	const float CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	FVector DownStart = WallHit.ImpactPoint - (WallHit.Normal * 15.0f);
	DownStart.Z = GetActorLocation().Z + CapsuleHalfHeight + LedgeGrabTraceHeight;
	FVector DownEnd = FVector(DownStart.X, DownStart.Y, GetActorLocation().Z);

	FHitResult LedgeHit;
	bool bHitLedge = GetWorld()->LineTraceSingleByChannel(LedgeHit, DownStart, DownEnd, ECC_WorldStatic, TraceParams);

	if (!bHitLedge || LedgeHit.bStartPenetrating || LedgeHit.Normal.Z < 0.7f)
	{
		return false;
	}

	OutLedgeLoc = FVector(WallHit.ImpactPoint.X, WallHit.ImpactPoint.Y, LedgeHit.ImpactPoint.Z);
	OutWallNormal = WallHit.Normal;
	return true;
}

void ABunny_Third_PersonCharacter::TickLedgeShimmy(float DeltaTime)
{
	if (FMath::Abs(CurrentMoveInput.X) < 0.15f)
	{
		return;
	}

	const float DirectionSign = FMath::Sign(CurrentMoveInput.X);

	// find if you at ledge
	FVector AheadLedgeLoc, NewWallNormal;
	if (!CanShimmy(DirectionSign, AheadLedgeLoc, NewWallNormal))
	{
		return;
	}

	const FVector WallRight = FVector::CrossProduct(LedgeWallNormal,FVector::UpVector).GetSafeNormal();
	const float MoveAmount = CurrentMoveInput.X * LedgeShimmySpeed * DeltaTime;
	const FVector DesiredMovement = WallRight * MoveAmount;

	AddActorWorldOffset(DesiredMovement, false);

	const float CapsuleRadius = GetCapsuleComponent()->GetScaledCapsuleRadius();
	const float CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	FCollisionQueryParams TraceParams(FName(TEXT("LedgeSnapTrace")), true, this);
	TraceParams.AddIgnoredActor(this);
	if (HeldGrabObject)
	{
		TraceParams.AddIgnoredActor(HeldGrabObject);
	}

	const FVector ActorLoc = GetActorLocation();
	const FVector ForwardStart = ActorLoc + FVector(0.0f, 0.0f, 30.0f);
	const FVector ForwardEnd = ForwardStart + (-LedgeWallNormal * (CapsuleRadius + 40.0f));

	FHitResult WallHit;
	if (GetWorld()->LineTraceSingleByChannel(WallHit, ForwardStart, ForwardEnd, ECC_WorldStatic, TraceParams))
	{
		if (FMath::Abs(WallHit.Normal.Z) <= 0.15f)
		{
			LedgeWallNormal = WallHit.Normal;

			FVector DownStart = WallHit.ImpactPoint - (WallHit.Normal * 15.0f);
			DownStart.Z = ActorLoc.Z + CapsuleHalfHeight + LedgeGrabTraceHeight;
			FVector DownEnd = FVector(DownStart.X, DownStart.Y, ActorLoc.Z - 30.0f);

			FHitResult LedgeHit;
			if (GetWorld()->LineTraceSingleByChannel(LedgeHit, DownStart, DownEnd, ECC_WorldStatic, TraceParams))
			{
				if (!LedgeHit.bStartPenetrating && LedgeHit.Normal.Z >= 0.7f)
				{
					LedgeLocation = FVector(WallHit.ImpactPoint.X, WallHit.ImpactPoint.Y, LedgeHit.ImpactPoint.Z);
				}
			}

			FVector SnappedLocation = WallHit.ImpactPoint + (LedgeWallNormal * (CapsuleRadius + 5.0f));
			SnappedLocation.Z = LedgeLocation.Z - LedgeHangVerticalOffset;
			SetActorLocation(SnappedLocation, false);

			FRotator FaceWall = (-LedgeWallNormal).Rotation();
			FaceWall.Pitch = 0.0f;
			FaceWall.Roll = 0.0f;
			SetActorRotation(FaceWall);
			return;
		}
	}
	FVector TargetPos = GetActorLocation();
	TargetPos.Z = LedgeLocation.Z - LedgeHangVerticalOffset;
	SetActorLocation(TargetPos, false);

	FRotator FaceWall = (-LedgeWallNormal).Rotation();
	FaceWall.Pitch = 0.0f;
	FaceWall.Roll = 0.0f;
	SetActorRotation(FaceWall);
}

void ABunny_Third_PersonCharacter::UpdateMovingBaseMovement()
{
	if (!MovingBaseComponent.IsValid())
	{
		return;
	}

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
