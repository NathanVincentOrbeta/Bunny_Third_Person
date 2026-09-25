#include "CombatComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/DamageEvents.h"

UCombatComponent::UCombatComponent() {

	PrimaryComponentTick.bCanEverTick = false;
}

void UCombatComponent::Attack() {

	// if attacking already, hop off code
	if (bIsAttacking) return;

	// Get the owner character and play the attack montage
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (!OwnerCharacter || !AttackMontage) return;

	//Animation
	OwnerCharacter -> PlayAnimMontage(AttackMontage);

	// Perform the attack trace to detect hits
	bIsAttacking = true;
	PerformAttackTrace();

	// Set a timer to reset the attack state after the cooldown
	GetWorld()->GetTimerManager().SetTimer(CooldownTimerHandle, this, &UCombatComponent::ResetAttack, AttackCooldown, false);
}

void UCombatComponent::PerformAttackTrace() {

	AActor* Owner = GetOwner();
	if (!Owner) return;

	FVector Start = Owner->GetActorLocation();
	FVector Forward = Owner->GetActorForwardVector();
	FVector End = Start + (Forward * AttackRange);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);

	bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult,
		Start,
		End,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(AttackRadius),
		Params
	);

	if (bHit && HitResult.GetActor()) {

		AActor* HitActor = HitResult.GetActor();

		// Apply damage to the hit actor if it implements the TakeDamage function
		UE_LOG(LogTemp, Log, TEXT("CombatComponent Hit: %s"), *HitResult.GetActor()->GetName());

		//HitActor->TakeDamage(20.0f, FDamageEvent(), GetOwner()->GetInstigatorController(), GetOwner());
		//AController* AttackController = GetOwner()->GetInstigatorController();

		APawn* InstigatorPawn = GetOwner()->GetInstigator();
		AController* Controller = InstigatorPawn ? InstigatorPawn->GetController() : nullptr;

		HitActor->TakeDamage(100.0f, FDamageEvent(), Controller, GetOwner());
	}
}

void UCombatComponent::ResetAttack() {

	bIsAttacking = false;
}