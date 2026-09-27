#include "BTTask_FindRandomPatrolPoint.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"

UBTTask_FindRandomPatrolPoint::UBTTask_FindRandomPatrolPoint() {

	NodeName = TEXT("Find Random Patrol Point");
}

EBTNodeResult::Type UBTTask_FindRandomPatrolPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) {

	AAIController* AIController = OwnerComp.GetOwner<AAIController>();
	if (!AIController) return EBTNodeResult::Failed;

	APawn* MyPawn = AIController->GetPawn();
	if (!MyPawn) return EBTNodeResult::Failed;

	UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
	FVector HomeLocation = BBComp->GetValueAsVector(TEXT("HomeLocation"));
	float PatrolRadius = BBComp->GetValueAsFloat(TEXT("PatrolRadius"));

	if (HomeLocation.IsNearlyZero()) {

		HomeLocation = MyPawn->GetActorLocation();
		BBComp->SetValueAsVector(TEXT("HomeLocation"), HomeLocation);
	}

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSys) return EBTNodeResult::Failed;

	FNavLocation RandomPoint;
	if (NavSys->GetRandomReachablePointInRadius(HomeLocation, PatrolRadius, RandomPoint)) {

		BBComp->SetValueAsVector(TEXT("TargetLocation"), RandomPoint.Location);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}
