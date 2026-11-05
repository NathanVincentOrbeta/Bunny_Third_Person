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

class APatrolRoute;

UENUM(BlueprintType)
enum class EAIPerceptionMode : uint8 {

	Stealth UMETA(DisplayName = "Stealth/Patrol"),
	Battle UMETA(DisplayName = "Battle/Omniscient")
};

UCLASS()
class BUNNY_THIRD_PERSON_API AAIC_NpcEnemy : public AAIController
{
	GENERATED_BODY()

public:
	AAIC_NpcEnemy();

	UFUNCTION(BlueprintCallable, Category = "AI|Perception")
	void SetPerceptionMode(EAIPerceptionMode NewMode);

protected:
	virtual void OnPossess(APawn* InPawn) override;
	void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = "AI")
	UBehaviorTree* PatrolBehaviorTree;

	UPROPERTY(EditAnywhere, Category = "AI")
	UBlackboardData* PatrolBlackboard;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UAIPerceptionComponent* PerceptionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UAISenseConfig_Sight* SightConfig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	EAIPerceptionMode CurrentPerceptionMode = EAIPerceptionMode::Stealth;

	UFUNCTION()
	void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);
	
};
