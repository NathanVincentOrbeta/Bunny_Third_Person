#include "RangedCombatComponent.h"
#include "GameFramework/Character.h"
#include "Engine/DamageEvents.h"
#include "DrawDebugHelpers.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

void URangedCombatComponent::PerformAttackTrace()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	// Start trace around chest height (+50 on Z)
	FVector Start = Owner->GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);
	FVector ShootDirection = Owner->GetActorForwardVector();

	// If the owner is an AI, aim directly at the TargetActor
	if (APawn* OwnerPawn = Cast<APawn>(Owner))
	{
		if (AAIController* AIController = Cast<AAIController>(OwnerPawn->GetController()))
		{
			if (UBlackboardComponent* BBComp = AIController->GetBlackboardComponent())
			{
				if (AActor* TargetActor = Cast<AActor>(BBComp->GetValueAsObject(TEXT("TargetActor"))))
				{
					// Aim straight at the player's center/chest
					FVector TargetLocation = TargetActor->GetActorLocation();
					ShootDirection = (TargetLocation - Start).GetSafeNormal();

					// Also ensure the body faces the player horizontally
					FVector FaceDirection = TargetLocation - Owner->GetActorLocation();
					FaceDirection.Z = 0.0f;
					if (!FaceDirection.IsNearlyZero())
					{
						Owner->SetActorRotation(FaceDirection.Rotation());
					}
				}
			}
		}
	}

	FVector End = Start + (ShootDirection * MaxRangedDistance);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);

	// Line trace for ranged attack
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECC_Pawn,
		Params
	);

	// Draw Debug Visuals
	if (bShowDebugTrace)
	{
		FVector TraceEnd = bHit ? HitResult.ImpactPoint : End;
		FColor LineColor = bHit ? FColor::Red : FColor::Green;

		DrawDebugLine(GetWorld(), Start, TraceEnd, LineColor, false, DebugTraceDuration, 0, 3.0f);

		if (bHit)
		{
			DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 16.0f, 12, FColor::Yellow, false, DebugTraceDuration, 0, 2.0f);
		}
	}

	if (bHit && HitResult.GetActor())
	{
		AActor* HitActor = HitResult.GetActor();
		UE_LOG(LogTemp, Log, TEXT("Ranged Attack Hit: %s"), *HitActor->GetName());

		APawn* InstigatorPawn = Owner->GetInstigator();
		AController* Controller = InstigatorPawn ? InstigatorPawn->GetController() : nullptr;

		HitActor->TakeDamage(RangedDamage, FDamageEvent(), Controller, Owner);
	}
}