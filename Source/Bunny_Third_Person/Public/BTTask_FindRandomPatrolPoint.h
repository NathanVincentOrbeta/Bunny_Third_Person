#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FindRandomPatrolPoint.generated.h"

/**
 * 
 */
UCLASS()
class BUNNY_THIRD_PERSON_API UBTTask_FindRandomPatrolPoint : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_FindRandomPatrolPoint();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
