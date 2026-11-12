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
	bool bIsMovingForward = BBComp->GetValueAsBool(TEXT("bIsMovingForward"));
	int32 NextIndex = CurrentIndex;

	if (Route->bPingPong) {

		// Checks next index based on direction
		NextIndex = bIsMovingForward ? CurrentIndex + 1 : CurrentIndex - 1;

		// Checks if we are at the end of the list of points
		if (NextIndex >= Route->Waypoints.Num()) {
			bIsMovingForward = false;
			NextIndex = Route->Waypoints.Num() - 2;
		}
		// Checks if we are at the start of the list of points
		else if (NextIndex < 0) {
			bIsMovingForward = true;
			NextIndex = 1;
		}

		// If there is only 1 point
		if (Route->Waypoints.Num() <= 1) NextIndex = 0;
	}
	else {

		NextIndex = (CurrentIndex + 1);
		if (NextIndex >= Route->Waypoints.Num()) {

			NextIndex = 0;
		}
	}

	NextIndex = FMath::Clamp(NextIndex, 0, Route->Waypoints.Num() - 1);

	FVector TargetLoc = Route->Waypoints[NextIndex]->GetActorLocation();
	BBComp->SetValueAsVector(TEXT("TargetLocation"), TargetLoc);
	BBComp->SetValueAsInt(TEXT("CurrentWaypointIndex"), NextIndex);
	BBComp->SetValueAsBool(TEXT("bIsMovingForward"), bIsMovingForward);

	return EBTNodeResult::Succeeded;
}

