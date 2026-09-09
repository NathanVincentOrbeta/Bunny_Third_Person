#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CombatComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BUNNY_THIRD_PERSON_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	UCombatComponent();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void Attack();

protected:
	
	void PerformAttackTrace();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackDamage = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackRange = 150.0F;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackCooldown = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	UAnimMontage* AttackMontage;

	float LastAttackTime = 0.0f;

};
