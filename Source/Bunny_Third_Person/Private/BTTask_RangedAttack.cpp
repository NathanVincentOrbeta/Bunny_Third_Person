#include "BTTask_RangedAttack.h"
#include "AIController.h"
#include "Npc.h"
#include "RangedCombatComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_RangedAttack::UBTTask_RangedAttack() {
	NodeName = TEXT("Ranged Attack");
}

EBTNodeResult::Type UBTTask_RangedAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) {

	AAIController* AIController = Cast<AAIController>(OwnerComp.GetOwner());
	if (!AIController) return EBTNodeResult::Failed;

	// Get Blackboard and Target
	UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
	if (!BBComp) return EBTNodeResult::Failed;

	AActor* TargetActor = Cast<AActor>(BBComp->GetValueAsObject(TEXT("TargetActor")));
	if (!TargetActor) return EBTNodeResult::Failed;

	// Line of Sight Check
	if (!AIController->LineOfSightTo(TargetActor)) {
		return EBTNodeResult::Failed;
	}

	// Get NPC and Combat Component
	ANpc* MyNpc = Cast<ANpc>(AIController->GetPawn());
	if (!MyNpc || !MyNpc->NPCCombatComp) return EBTNodeResult::Failed;

	// Rotate to face target
	FVector DirectionToTarget = TargetActor->GetActorLocation() - MyNpc->GetActorLocation();
	DirectionToTarget.Z = 0.0f;

	if (!DirectionToTarget.IsNearlyZero()) {
		MyNpc->SetActorRotation(DirectionToTarget.Rotation());
	}

	// Attack
	MyNpc->NPCCombatComp->Attack();
	return EBTNodeResult::Succeeded;
}