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
#include <BP_ObjectGrab.h>

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

ABunny_Third_PersonCharacter::ABunny_Third_PersonCharacter()
{
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
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

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

void ABunny_Third_PersonCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ABunny_Third_PersonCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ABunny_Third_PersonCharacter::DoJumpEnd);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABunny_Third_PersonCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ABunny_Third_PersonCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABunny_Third_PersonCharacter::Look);

		// Grab
		EnhancedInputComponent->BindAction(GrabAction, ETriggerEvent::Started, this, & ABunny_Third_PersonCharacter::DoGrab);
	}
	else
	{
		UE_LOG(LogBunny_Third_Person, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ABunny_Third_PersonCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
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

		// Posistion the box 
		const FVector DropLocation = FVector(
			PlayerLocation.X,
			PlayerLocation.Y,
			CapsuleBottomZ - BoxExtent.Z - 10.0f
		);

		FVector DownwardImpulse = FVector(0.0f, 0.0f, -ObjectDownImpulse); // hold
		HeldGrabObject->PlaceAndDrop(DropLocation, DownwardImpulse); //HeldGrabObject->DropAndImpulse(DownwardImpulse); ** Remove Test add this
		/**
		Test code end
		*/

		// Clear hand 
		HeldGrabObject = nullptr;

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
