// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_MeleeAttack.h"
#include "AIController.h"
#include "Npc.h"
#include "CombatComponent.h"

UBTTask_MeleeAttack::UBTTask_MeleeAttack() {
	NodeName = TEXT("Melee Attack");
}

EBTNodeResult::Type UBTTask_MeleeAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) {

	AAIController* AIController = Cast<AAIController>(OwnerComp.GetOwner());
	if (!AIController) return EBTNodeResult::Failed;

	ANpc* MyNpc = Cast<ANpc>(AIController->GetPawn());
	UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();

	if (MyNpc && BBComp && MyNpc->NPCCombatComp) {

		AActor* Target = Cast<AActor>(BBComp->GetValueAsObject(TEXT("TargetActor")));
		if (Target) {

			float Distance = FVector::Dist(MyNpc->GetActorLocation(), Target->GetActorLocation());

			// Distance check
			if (Distance > MyNpc->NPCCombatComp->AttackRange + 50.0f) {

				return EBTNodeResult::Failed;
			}
		}
	}

	if (MyNpc && MyNpc->NPCCombatComp) {

		MyNpc->NPCCombatComp->Attack();
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}

