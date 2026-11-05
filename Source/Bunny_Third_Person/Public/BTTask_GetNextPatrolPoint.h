#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_GetNextPatrolPoint.generated.h"

/**
 * 
 */
UCLASS()
class BUNNY_THIRD_PERSON_API UBTTask_GetNextPatrolPoint : public UBTTaskNode
{
	GENERATED_BODY()

public:

	UBTTask_GetNextPatrolPoint();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
