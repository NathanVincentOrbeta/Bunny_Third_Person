#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.h"
#include "RangedCombatComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BUNNY_THIRD_PERSON_API URangedCombatComponent : public UCombatComponent
{
	GENERATED_BODY()

protected:
	// Override the trace logic to be a long line instead of a small sphere
	virtual void PerformAttackTrace() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ranged Combat")
	float RangedDamage = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ranged Combat")
	float MaxRangedDistance = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ranged Combat | Debug")
	bool bShowDebugTrace = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ranged Combat | Debug")
	float DebugTraceDuration = 1.5f;
};
