#include "AIC_NpcEnemy.h"
#include "Npc.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AIPerceptionTypes.h"
#include "Bunny_Third_PersonCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "PatrolRoute.h"

AAIC_NpcEnemy::AAIC_NpcEnemy() {
	PrimaryActorTick.bCanEverTick = true;

	PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

	// Configure Sight
	SightConfig->SightRadius = 500.0f;
	SightConfig->LoseSightRadius = 1000.0f;
	SightConfig->PeripheralVisionAngleDegrees = 270.0f;
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

	PerceptionComp->ConfigureSense(*SightConfig);
	PerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
}

void AAIC_NpcEnemy::SetPerceptionMode(EAIPerceptionMode NewMode) {

	CurrentPerceptionMode = NewMode;

	if (CurrentPerceptionMode == EAIPerceptionMode::Battle) {

		UE_LOG(LogTemp, Warning, TEXT("AI: Switching to Battle Mode! Player location is now known."));
	}
	else {

		UE_LOG(LogTemp, Warning, TEXT("AI: Switching to Patrol Mode. Relying on sight"));
	}
}

void AAIC_NpcEnemy::Tick(float DeltaTime) {

	Super::Tick(DeltaTime);

	if (CurrentPerceptionMode == EAIPerceptionMode::Battle) {

		if (Blackboard) {

			AActor* PlayerActor = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

			if (PlayerActor) {

				Blackboard->SetValueAsObject(TEXT("TargetActor"), PlayerActor);

				if (ANpc* MyNpc = Cast<ANpc>(GetPawn())) {

					MyNpc->SetMovementSpeed(true);
				}
			}
		}
	}
}

void AAIC_NpcEnemy::OnPossess(APawn* InPawn) {

	Super::OnPossess(InPawn);

	if (PatrolBlackboard) {

		UBlackboardComponent* TempBlackboard = nullptr;
		UseBlackboard(PatrolBlackboard, TempBlackboard);
		Blackboard = TempBlackboard;
	}

	if (Blackboard) {
		// 1. Cast the InPawn to our ANpc class to access its variables
		ANpc* MyNpc = Cast<ANpc>(InPawn);
		if (MyNpc && MyNpc->AssignedRoute) {
			// 2. Take the route from the NPC and put it into the Blackboard
			Blackboard->SetValueAsObject(TEXT("PatrolRoute"), MyNpc->AssignedRoute);
			Blackboard->SetValueAsInt(TEXT("CurrentWaypointIndex"), 0);
			UE_LOG(LogTemp, Warning, TEXT("AI: Assigned to route %s"), *MyNpc->AssignedRoute->GetName());
		}
		else {
			UE_LOG(LogTemp, Error, TEXT("AI: This NPC has no Assigned Route!"));
		}
	}

	if (PatrolBehaviorTree) {

		RunBehaviorTree(PatrolBehaviorTree);
	}

	PerceptionComp->OnPerceptionUpdated.AddDynamic(this, &AAIC_NpcEnemy::OnPerceptionUpdated);
}

void AAIC_NpcEnemy::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors) {

	if(CurrentPerceptionMode == EAIPerceptionMode::Battle) return;

	for (AActor* Actor : UpdatedActors) {
		if (Actor && Actor->IsA(ABunny_Third_PersonCharacter::StaticClass())) {

			// Array to hold perceived actors.
			TArray<AActor*> CurrentlyPerceivedActors;
			PerceptionComp->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), CurrentlyPerceivedActors);

			ANpc* MyNpc = Cast<ANpc>(GetPawn());

			if (CurrentlyPerceivedActors.Contains(Actor)) {

				// If player is seen
				if (Blackboard) {
					Blackboard->SetValueAsObject(TEXT("TargetActor"), Actor);
					UE_LOG(LogTemp, Warning, TEXT("AI: Player spotted! Violating player..."));
				}

				if (MyNpc) { MyNpc->SetMovementSpeed(true); }
			}
			else {

				// If player is not seen
				if (Blackboard) {
					Blackboard->ClearValue(TEXT("TargetActor"));
					UE_LOG(LogTemp, Warning, TEXT("AI: Lost sight of player! Returning to gooning..."));
				}

				if (MyNpc) { MyNpc->SetMovementSpeed(false); }
			}
		}
	}
}
