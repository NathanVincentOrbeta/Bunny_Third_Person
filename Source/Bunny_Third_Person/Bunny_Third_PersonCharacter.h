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
class UCombatComponent; // Nathan Combat Component forward Declaration

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

	// Nathan CombatComponent
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCombatComponent* CombatComp;

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

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* AttackAction;

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

	void DoAttack();

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
	float DashDuration = 0.35f;

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

	/** Sprint Input Values */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprint")
	float SprintSpeed = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprint")
	float WalkSpeed = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprint", meta = (ClampMin = "0.1", ClampMax = "0.5"))
	float HoldToRunThresHold = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sprint")
	bool bIsSprinting = false;

	FTimerHandle HoldToRunTimerHandle;

	/** All wall logic values */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall Jump")
	bool bCanWallJump = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall Jump")
	bool bIsWallSliding = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall Jump")
	bool bIsWallStick = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall Jump")
	bool bHasWallStick = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Jump")
	float WallStickDuration = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Jump")
	float WallSlideSpeed = 160.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Jump")
	float WallJumpHorizontalForce = 650.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wall Jump")
	float WallJumpVerticalForce = 850.0f;

	FVector WallNormal; 
	FTimerHandle WallStickTimeHandle;
	bool bJustWallJumped = false;

	bool IsPushingIntoWall() const;

	/** Ledge Grab Properties */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ledge Grab")
	bool bIsLedgeHanging = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ledge Grab")
	bool bCanLedgeGrab = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Grab")
	float LedgeGrabForwardReach = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Grab")
	float LedgeGrabTraceHeight = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Grab")
	float LedgeHangVerticalOffset = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ledge Grab")
	float LedgeGrabCooldown = 0.4f;

	FTimerHandle LedgeCooldownTimerHandle;

	FVector LedgeLocation;
	FVector LedgeWallNormal; 

	bool DetectLedge(FVector& OutLedgeLoc, FVector& OutWallNormal);

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

	/** Handles Wall logics */
	UFUNCTION(BlueprintCallable, Category = "Input")
	virtual void DoWallJumpLedge();
	void StartWallSlide(const FVector& HitNormal);
	void StopWallSlide();
	void StartWallStick();
	void OnWallStickEnd();

	/** Run handles */
	void OnDashButtonPressed();
	void OnDashButtonReleased();

	void StartSprint();
	void StopSprint();

	/** Handle overlaping for object */
	void SetNearbyGrabObject(ABP_ObjectGrab* Object) { NearbyGrabObject = Object; }
	void ClearNearbyGrabObject(ABP_ObjectGrab* Object) { if (NearbyGrabObject == Object) NearbyGrabObject = nullptr; }

	/** Ledge Function */
	void StartLedgeGrab(const FVector& InLedgeLoc, const FVector& InWallNormal);
	void DropFromLedge();
	void ClimbUpLedge();
	void ResetLedgeGrabCooldown();

	UFUNCTION(BlueprintPure, Category = "Ledge Grab")
	bool IsLedgeHanging() const { return bIsLedgeHanging; }

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	/** Returns objectGrabable **/
	FORCEINLINE class USceneComponent* GetHoldPoint() const { return HoldPoint; }
};

