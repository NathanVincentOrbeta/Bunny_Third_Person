#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BUNNY_THIRD_PERSON_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:	
	// Default Values
	UCombatComponent();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void Attack();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	UAnimMontage* AttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackCooldown = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackRange = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackRadius = 50.0f;

	bool bIsAttacking = false;
	FTimerHandle CooldownTimerHandle;

	void PerformAttackTrace();

	void ResetAttack();

};
