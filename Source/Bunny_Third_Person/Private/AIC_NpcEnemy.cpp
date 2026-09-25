#include "AIC_NpcEnemy.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"

AAIC_NpcEnemy::AAIC_NpcEnemy() {}

void AAIC_NpcEnemy::OnPossess(APawn* InPawn) {

	Super::OnPossess(InPawn);

	if (PatrolBlackboard) {

		UBlackboardComponent* TempBlackboard = nullptr;

		UseBlackboard(PatrolBlackboard, TempBlackboard);

		Blackboard = TempBlackboard;
	}

	if (PatrolBehaviorTree) {

		RunBehaviorTree(PatrolBehaviorTree);
	}
}
