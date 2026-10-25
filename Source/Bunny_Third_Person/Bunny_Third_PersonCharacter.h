#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Bunny_Third_PersonCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class ABP_ObjectGrab;
class UCombatComponent; // Nathan Combat Component forward Declaration
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

public:
	ABunny_Third_PersonCharacter();

	virtual void Tick(float DeltaTime) override;
	virtual void Landed(const FHitResult& Hit) override;

	// ~ Movement State Queries
	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsLedgeHanging() const { return bIsLedgeHanging; }

	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsGroundPounding() const { return bIsGroundPounding; }

	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsSlopeSliding() const { return bIsSlopeSliding; }

	// ~ Object Interaction
	void SetNearbyGrabObject(ABP_ObjectGrab* Object) { NearbyGrabObject = Object; }
	void ClearNearbyGrabObject(ABP_ObjectGrab* Object) { if (NearbyGrabObject == Object) NearbyGrabObject = nullptr; }

	// ~ Subobject Getters
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE USceneComponent* GetHoldPoint() const { return HoldPoint; }

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// -------------------------------------------------------------------------
	// Components
	// -------------------------------------------------------------------------
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grab", meta = (AllowPrivateAccess = "true"))
	USceneComponent* HoldPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UCombatComponent* CombatComp;

	// -------------------------------------------------------------------------
	// Input Actions
	// -------------------------------------------------------------------------
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MouseLookAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* GrabAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* DashAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* AttackAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* CrouchAction;

	// -------------------------------------------------------------------------
	// Movement / General
	// -------------------------------------------------------------------------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Sprint")
	float SprintSpeed = 900.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Sprint")
	float WalkSpeed = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Sprint", meta = (ClampMin = "0.1", ClampMax = "0.5"))
	float HoldToRunThreshold = 0.2f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Sprint")
	bool bIsSprinting = false;

	// -------------------------------------------------------------------------
	// Dash
	// -------------------------------------------------------------------------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Dash", meta = (ClampMin = "500.0"))
	float DashSpeed = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Dash", meta = (ClampMin = "0.05", ClampMax = "0.5"))
	float DashDuration = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Dash", meta = (ClampMin = "0.0"))
	float DashCooldown = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Dash")
	float DashExitSpeed = 800.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Dash")
	bool bIsDashing = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Dash")
	bool bCanAirDash = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Dash")
	bool bDashOnCoolDown = false;

	// -------------------------------------------------------------------------
	// Wall Jump & Slide
	// -------------------------------------------------------------------------
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Wall")
	bool bCanWallJump = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Wall")
	bool bIsWallSliding = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Wall")
	bool bIsWallStick = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Wall")
	bool bHasWallStick = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Wall")
	float WallStickDuration = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Wall")
	float WallSlideSpeed = 160.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Wall")
	float WallJumpHorizontalForce = 650.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Wall")
	float WallJumpVerticalForce = 850.0f;

	// -------------------------------------------------------------------------
	// Ledge Grab
	// -------------------------------------------------------------------------
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Ledge")
	bool bIsLedgeHanging = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Ledge")
	bool bCanLedgeGrab = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Ledge")
	float LedgeGrabForwardReach = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Ledge")
	float LedgeGrabTraceHeight = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Ledge")
	float LedgeHangVerticalOffset = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Ledge")
	float LedgeGrabCooldown = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Ledge")
	float LedgeShimmySpeed = 200.0f;

	// -------------------------------------------------------------------------
	// Crouch & Ground Pound
	// -------------------------------------------------------------------------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Crouch")
	float DoubleTapCrouchThreshold = 0.28f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Ground Pound")
	bool bIsGroundPounding = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Ground Pound")
	float GroundPoundSpeed = 2600.0f;

	// -------------------------------------------------------------------------
	// Slope Slide
	// -------------------------------------------------------------------------
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement|Slope Slide")
	bool bIsSlopeSliding = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slope Slide")
	float MinSlopeSlideAngle = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slope Slide")
	float SlopeSlideAcceleration = 2200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement|Slope Slide")
	float MaxSlopeSlideSpeed = 1600.0f;

	// -------------------------------------------------------------------------
	// Objects & Grab
	// -------------------------------------------------------------------------
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Grab")
	ABP_ObjectGrab* HeldGrabObject = nullptr;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Grab")
	ABP_ObjectGrab* NearbyGrabObject = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Jump")
	float ObjectJumpBoost = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grab|Jump")
	float ObjectDownImpulse = 500.0f;

	// -------------------------------------------------------------------------
	// Input Callbacks & Movement Functions
	// -------------------------------------------------------------------------
	UFUNCTION(BlueprintCallable, Category = "Movement|Input")
	virtual void DoMove(float Right, float Forward);

	UFUNCTION(BlueprintCallable, Category = "Movement|Input")
	virtual void DoLook(float Yaw, float Pitch);

	UFUNCTION(BlueprintCallable, Category = "Movement|Input")
	virtual void DoJumpStart();

	UFUNCTION(BlueprintCallable, Category = "Movement|Input")
	virtual void DoJumpEnd();

	UFUNCTION(BlueprintCallable, Category = "Movement|Input")
	virtual void DoCrouch();

	UFUNCTION(BlueprintCallable, Category = "Movement|Input")
	virtual void DoGrab();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void DoAttack();

	UFUNCTION(BlueprintCallable, Category = "Movement|Dash")
	void StartDash();

	UFUNCTION(BlueprintCallable, Category = "Movement|Dash")
	void StopDash();

	UFUNCTION(BlueprintCallable, Category = "Movement|Ground Pound")
	void StartGroundPound();

	UFUNCTION(BlueprintCallable, Category = "Movement|Ground Pound")
	void StopGroundPound();

	void StartSprint();
	void StopSprint();

	void StartWallSlide(const FVector& HitNormal, UPrimitiveComponent* HitComponent = nullptr);
	void StopWallSlide();
	void StartWallStick();
	void OnWallStickEnd();
	bool TryWallJump();

	void StartLedgeGrab(const FVector& InLedgeLoc, const FVector& InWallNormal, UPrimitiveComponent* HitComponent = nullptr);
	void DropFromLedge();
	void ClimbUpLedge();

private:
	// Input handlers
	void Move(const FInputActionValue& Value);
	void MoveCompleted(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void OnDashButtonPressed();
	void OnDashButtonReleased();

	// Ticks & Helpers
	void TickSlopeSlide(float DeltaTime);
	void TickLedgeShimmy(float DeltaTime);
	void UpdateMovingBaseMovement();
	bool IsPushingIntoWall() const;
	bool CanShimmy(float DirectionSign, FVector& OutLedgeLoc, FVector& OutWallNormal);
	bool DetectLedge(FVector& OutLedgeLoc, FVector& OutWallNormal, UPrimitiveComponent*& OutHitComponent);

	/** Shared two-ray trace for detecting ledge forward and surface downward */
	bool TraceLedgeSurface(const FVector& RayStart, const FVector& ForwardDir, float ForwardDist, FHitResult& OutWallHit, FHitResult& OutLedgeHit) const;

	void ResetDashCooldown();
	void ResetLedgeGrabCooldown();

	// Internal state tracking
	FVector2D CurrentMoveInput = FVector2D::ZeroVector;
	FVector DashDirection = FVector::ZeroVector;
	FVector WallNormal = FVector::ZeroVector;
	FVector LedgeLocation = FVector::ZeroVector;
	FVector LedgeWallNormal = FVector::ZeroVector;

	float DefaultGravityScale = 1.75f;
	float DefaultBrakingDecelWalking = 2000.0f;
	float LastCrouchPressTime = -1.0f;

	FTransform PreviousBaseTransform;
	TWeakObjectPtr<UPrimitiveComponent> MovingBaseComponent;

	FTimerHandle DashTimeHandle;
	FTimerHandle DashCooldownTimerHandle;
	FTimerHandle HoldToRunTimerHandle;
	FTimerHandle WallStickTimeHandle;
	FTimerHandle LedgeCooldownTimerHandle;
};