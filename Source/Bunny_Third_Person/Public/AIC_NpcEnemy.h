#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AIC_NpcEnemy.generated.h"

/**
 * 
 */
UCLASS()
class BUNNY_THIRD_PERSON_API AAIC_NpcEnemy : public AAIController
{
	GENERATED_BODY()

public:
	AAIC_NpcEnemy();

protected:
	virtual void OnPossess(APawn* InPawn) override;

	UPROPERTY(EditAnywhere, Category = "AI")
	UBehaviorTree* PatrolBehaviorTree;

	UPROPERTY(EditAnywhere, Category = "AI")
	UBlackboardData* PatrolBlackboard;
	
};
