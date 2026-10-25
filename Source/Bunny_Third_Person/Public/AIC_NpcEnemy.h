#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AIPerceptionTypes.h"
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UAIPerceptionComponent* PerceptionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UAISenseConfig_Sight* SightConfig;

	UFUNCTION()
	void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);
	
};
