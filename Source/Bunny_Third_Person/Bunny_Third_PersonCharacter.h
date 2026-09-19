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


public:

	/** Constructor */
	ABunny_Third_PersonCharacter();	

protected:

	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	UPROPERTY(VisibleInstanceOnly, Category = "Grab")
	ABP_ObjectGrab* HeldGrabObject;

	UPROPERTY(VisibleInstanceOnly, Category = "Grab")
	ABP_ObjectGrab* NearbyGrabObject;

	/** Momentum for the object jump */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jump", meta = (AllowAbstract = "true"))
	float ObjectJumpBoost = 700.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, CateGOry = "Jump", meta = (AllowAbstract = "true"))
	float ObjectDownImpulse = 800.0f;

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

