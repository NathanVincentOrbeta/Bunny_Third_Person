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


DEFINE_LOG_CATEGORY(LogTemplateCharacter);

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

	if (bIsDashing)
	{
		GetCharacterMovement()->Velocity = DashDirection * DashSpeed;
	}
}

void ABunny_Third_PersonCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
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

void ABunny_Third_PersonCharacter::StartDash()
{
	if (bIsDashing || bDashOnCoolDown) return;

	const bool bIsGrounded = GetCharacterMovement()->IsMovingOnGround();

	// only one air dash
	if (!bIsGrounded)
	{
		if (!bCanAirDash) return;
		bCanAirDash = false;
	}

	// cheack if player press (WASD)
	if (!CurrentMoveInput.IsNearlyZero()) 
	{
		FRotator ControlRot = Controller ? Controller->GetControlRotation() : GetActorRotation();
		FRotator YawRot(0.f, ControlRot.Yaw, 0.f);

		const FVector CameraFoward = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
		const FVector CameraRight = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

		DashDirection = (CameraFoward * CurrentMoveInput.Y) + (CameraRight * CurrentMoveInput.X);
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

	bCanAirDash = true;
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
	if (!GetCharacterMovement()->IsMovingOnGround())
	{
		return;
	}

	bIsSprinting = true;
	GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
}

void ABunny_Third_PersonCharacter::StopSprint()
{
	bIsSprinting = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

