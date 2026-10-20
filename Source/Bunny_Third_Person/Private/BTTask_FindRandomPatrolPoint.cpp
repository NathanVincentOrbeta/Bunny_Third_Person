#include "BTTask_FindRandomPatrolPoint.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"

UBTTask_FindRandomPatrolPoint::UBTTask_FindRandomPatrolPoint() {

	NodeName = TEXT("Find Random Patrol Point");
}

EBTNodeResult::Type UBTTask_FindRandomPatrolPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) {

	//Debug Log
	UE_LOG(LogTemp, Warning, TEXT("BTTask: Step 1 - Task Started"));

	AAIController* AIController = Cast<AAIController>(OwnerComp.GetOwner());
	if (!AIController) {

		UE_LOG(LogTemp, Error, TEXT("BTTask: Step 2 - No AI Controller!"));
		return EBTNodeResult::Failed;
	}

	APawn* MyPawn = AIController->GetPawn();
	if (!MyPawn) {

		UE_LOG(LogTemp, Error, TEXT("BTTask: Step 3 - No Pawn!"));
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
	if (!BBComp) {

		UE_LOG(LogTemp, Error, TEXT("BTTask: Step 3 - No Blackboard!"));
		return EBTNodeResult::Failed;

	}

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys) {

		UE_LOG(LogTemp, Error, TEXT("BTTask: Step 6 - NavSys is NULL!"));
		return EBTNodeResult::Failed;
	}

	FNavLocation RandomPoint;
	if (NavSys->GetRandomReachablePointInRadius(MyPawn->GetActorLocation(), 10000.0f, RandomPoint)) {

		BBComp->SetValueAsVector(TEXT("TargetLocation"), RandomPoint.Location);
		UE_LOG(LogTemp, Warning, TEXT("BTTask: SUCCESS! Target set to: %s"), *RandomPoint.Location.ToString());
		return EBTNodeResult::Succeeded;
	}

	UE_LOG(LogTemp, Error, TEXT("BTTask: Step 7 - NavSys could not find a point in radius!"));
	return EBTNodeResult::Failed;
}
