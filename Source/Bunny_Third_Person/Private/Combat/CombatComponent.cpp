#include "Combat/CombatComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
UCombatComponent::UCombatComponent()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryComponentTick.bCanEverTick = false;

}

void UCombatComponent::Attack() {

	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastAttackTime < AttackCooldown) return;

	LastAttackTime = CurrentTime;

	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	if (OwnerChar && AttackMontage) {

		OwnerChar->PlayAnimMontage(AttackMontage);
	}

	PerformAttackTrace();
}



void UCombatComponent::PerformAttackTrace() {

	AActor* Owner = GetOwner();
	if (!Owner) return;

	FVector Start = Owner->GetActorLocation();
	FVector End = Start + (Owner->GetActorForwardVector() * AttackRange);

	FCollisionShape Sphere = FCollisionShape::MakeSphere(50.0f);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);

	bool bHit = GetWorld()->SweepSingleByChannel(
		HitResult,
		Start,
		End,
		FQuat::Identity,
		ECC_Pawn,
		Sphere,
		Params
	);

	if (bHit && HitResult.GetActor()) {

		UGameplayStatics::ApplyDamage(
			HitResult.GetActor(),
			AttackDamage,
			Owner->GetInstigatorController(),
			Owner,
			UDamageType::StaticClass()
		);
	}
}