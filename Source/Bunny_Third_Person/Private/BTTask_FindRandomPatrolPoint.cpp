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
	if (!AIController) return EBTNodeResult::Failed;
	UE_LOG(LogTemp, Error, TEXT("BTTask: Step 2 - No AI Controller!"));

	APawn* MyPawn = AIController->GetPawn();
	if (!MyPawn) return EBTNodeResult::Failed;
	UE_LOG(LogTemp, Error, TEXT("BTTask: Step 3 - No Pawn!"));

	UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
	FVector HomeLocation = BBComp->GetValueAsVector(TEXT("HomeLocation"));
	float PatrolRadius = BBComp->GetValueAsFloat(TEXT("PatrolRadius"));

	UE_LOG(LogTemp, Log, TEXT("BTTask: Step 4 - Home: %s, Radius: %f"), *HomeLocation.ToString(), PatrolRadius);

	if (HomeLocation.IsNearlyZero()) {

		HomeLocation = MyPawn->GetActorLocation();
		BBComp->SetValueAsVector(TEXT("HomeLocation"), HomeLocation);
		UE_LOG(LogTemp, Log, TEXT("BTTask: Step 5 - Set HomeLocation to current pos"));
	}

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (!NavSys) return EBTNodeResult::Failed;
	UE_LOG(LogTemp, Error, TEXT("BTTask: Step 6 - NavSys is NULL!"));

	FNavLocation RandomPoint;
	if (NavSys->GetRandomReachablePointInRadius(HomeLocation, PatrolRadius, RandomPoint)) {

		BBComp->SetValueAsVector(TEXT("TargetLocation"), RandomPoint.Location);
		UE_LOG(LogTemp, Warning, TEXT("BTTask: SUCCESS! Target set to: %s"), *RandomPoint.Location.ToString());
		return EBTNodeResult::Succeeded;
	}

	UE_LOG(LogTemp, Error, TEXT("BTTask: Step 7 - NavSys could not find a point in radius!"));

	return EBTNodeResult::Failed;
}
