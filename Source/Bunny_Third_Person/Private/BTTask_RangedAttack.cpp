#include "BTTask_RangedAttack.h"
#include "AIController.h"
#include "Npc.h"
#include "RangedCombatComponent.h"

UBTTask_RangedAttack::UBTTask_RangedAttack() {
	NodeName = TEXT("Ranged Attack");
}

EBTNodeResult::Type UBTTask_RangedAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) {
	AAIController* AIController = Cast<AAIController>(OwnerComp.GetOwner());
	if (!AIController) return EBTNodeResult::Failed;

	ANpc* MyNpc = Cast<ANpc>(AIController->GetPawn());
	if (MyNpc && MyNpc->NPCCombatComp) {
		MyNpc->NPCCombatComp->Attack();
		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::Failed;
}