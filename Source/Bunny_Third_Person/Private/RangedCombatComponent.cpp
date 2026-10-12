#include "RangedCombatComponent.h"
#include "GameFramework/Character.h"
#include "Engine/DamageEvents.h"

void URangedCombatComponent::PerformAttackTrace()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	FVector Start = Owner->GetActorLocation();
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

	if (bHit && HitResult.GetActor())
	{
		AActor* HitActor = HitResult.GetActor();
		UE_LOG(LogTemp, Log, TEXT("Ranged Attack Hit: %s"), *HitActor->GetName());

		APawn* InstigatorPawn = Owner->GetInstigator();
		AController* Controller = InstigatorPawn ? InstigatorPawn->GetController() : nullptr;

		HitActor->TakeDamage(RangedDamage, FDamageEvent(), Controller, Owner);
	}
}