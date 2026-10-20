#include "RangedCombatComponent.h"
#include "GameFramework/Character.h"
#include "Engine/DamageEvents.h"
#include "DrawDebugHelpers.h"

void URangedCombatComponent::PerformAttackTrace()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	FVector Start = Owner->GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);
	FVector Forward = Owner->GetActorForwardVector();
	FVector End = Start + (Forward * MaxRangedDistance);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);

	// Use a LineTrace for ranged attacks
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECC_Pawn,
		Params
	);
	if (bShowDebugTrace) {

		FVector TraceEnd = bHit ? HitResult.ImpactPoint : End;
		FColor LineColor = bHit ? FColor::Red : FColor::Green;

		DrawDebugLine(
			GetWorld(),
			Start,
			TraceEnd,
			LineColor,
			false,
			DebugTraceDuration,
			0,
			3.0f
		);

		if (bHit) {

			DrawDebugSphere(
				GetWorld(),
				HitResult.ImpactPoint,
				16.0f,
				12, FColor::Yellow,
				false,
				DebugTraceDuration,
				0,
				2.0f
			);
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