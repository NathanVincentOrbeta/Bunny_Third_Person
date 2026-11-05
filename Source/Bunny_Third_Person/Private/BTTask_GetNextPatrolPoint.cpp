#include "BTTask_GetNextPatrolPoint.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "PatrolRoute.h"

UBTTask_GetNextPatrolPoint::UBTTask_GetNextPatrolPoint() {

	NodeName = TEXT("Get Next Patrol Point");
}

EBTNodeResult::Type UBTTask_GetNextPatrolPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) {

	UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
	if (!BBComp) return EBTNodeResult::Failed;

	APatrolRoute* Route = Cast<APatrolRoute>(BBComp->GetValueAsObject(TEXT("PatrolRoute")));
	if (!Route || Route->Waypoints.Num() == 0) {
		UE_LOG(LogTemp, Error, TEXT("BTTask: No Patrol Route assigned or Route has no waypoints!"));
		return EBTNodeResult::Failed;
	}

	int32 CurrentIndex = BBComp->GetValueAsInt(TEXT("CurrentWaypointIndex"));
	int32 NextIndex = CurrentIndex + 1;

	if (Route->bPingPong) {

		if (NextIndex >= Route->Waypoints.Num()) {

			NextIndex = 0;
		}
	}
	else {

		if (NextIndex >= Route->Waypoints.Num()) {

			NextIndex = 0;
		}
	}

	FVector TargetLoc = Route->Waypoints[NextIndex]->GetActorLocation();
	BBComp->SetValueAsVector(TEXT("TargetLocation"), TargetLoc);
	BBComp->SetValueAsInt(TEXT("CurrentWaypointIndex"), NextIndex);

	return EBTNodeResult::Succeeded;
}

