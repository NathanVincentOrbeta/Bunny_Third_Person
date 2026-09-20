// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Bunny_Third_PersonCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class ABP_ObjectGrab;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS(abstract)
class ABunny_Third_PersonCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* HoldPoint;

protected:
	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* MouseLookAction;

	/** Grab Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* GrabAction;

	/** Dash Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* DashAction;


public:

	/** Constructor */
	ABunny_Third_PersonCharacter();	

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** movement input ends */
	void MoveCompleted(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	UPROPERTY(VisibleInstanceOnly, Category = "Grab")
	ABP_ObjectGrab* HeldGrabObject;

	UPROPERTY(VisibleInstanceOnly, Category = "Grab")
	ABP_ObjectGrab* NearbyGrabObject;

	/** Momentum for the object jump */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump", meta = (AllowAbstract = "true"))
	float ObjectJumpBoost = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump", meta = (AllowAbstract = "true"))
	float ObjectDownImpulse = 500.0f;

	/** Dash Input Values */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash", meta = (ClampMin = "500.0"))
	float DashSpeed = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash", meta = (ClampMin = "0.05", ClampMax = "0.5"))
	float DashDuration = 0.35;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash", meta = (ClampMin = "0.0"))
	float DashCooldown = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	float DashExitSpeed = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	bool bIsDashing = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	bool bCanAirDash = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dash")
	bool bDashOnCoolDown = false;

	FVector2D CurrentMoveInput;
	FVector DashDirection;
	float DefaultGravityScale;
	FTimerHandle DashTimeHandle;
	FTimerHandle DashCooldownTimerHandle;

public:

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

	/** Grab input */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoGrab();

	/** Handles Dash Input */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void StartDash();
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void StopDash();
	UFUNCTION()
	void ResetDashCooldown();
	virtual void Landed(const FHitResult& Hit) override;

	/** Handle overlaping for object */
	void SetNearbyGrabObject(ABP_ObjectGrab* Object) { NearbyGrabObject = Object; }
	void ClearNearbyGrabObject(ABP_ObjectGrab* Object) { if (NearbyGrabObject == Object) NearbyGrabObject = nullptr; }

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	/** Returns objectGrabable **/
	FORCEINLINE class USceneComponent* GetHoldPoint() const { return HoldPoint; }
};

