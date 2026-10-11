#include "AIC_NpcEnemy.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AIPerceptionTypes.h"
#include "Bunny_Third_PersonCharacter.h"

AAIC_NpcEnemy::AAIC_NpcEnemy() {

	PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

	// Configure Sight
	SightConfig->SightRadius = 500.0f;
	SightConfig->LoseSightRadius = 1000.0f;
	SightConfig->PeripheralVisionAngleDegrees = 90.0f;
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

	PerceptionComp->ConfigureSense(*SightConfig);
	PerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
}

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

	PerceptionComp->OnPerceptionUpdated.AddDynamic(this, &AAIC_NpcEnemy::OnPerceptionUpdated);
}

void AAIC_NpcEnemy::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors) {
	for (AActor* Actor : UpdatedActors) {
		if (Actor && Actor->IsA(ABunny_Third_PersonCharacter::StaticClass())) {

			// Array to hold perceived actors.
			TArray<AActor*> CurrentlyPerceivedActors;
			PerceptionComp->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), CurrentlyPerceivedActors);

			if (CurrentlyPerceivedActors.Contains(Actor)) {

				// If player is seen
				if (Blackboard) {
					Blackboard->SetValueAsObject(TEXT("TargetActor"), Actor);
					UE_LOG(LogTemp, Warning, TEXT("AI: Player spotted! Violating player..."));
				}
			}
			else {

				// If player is not seen
				if (Blackboard) {
					Blackboard->ClearValue(TEXT("TargetActor"));
					UE_LOG(LogTemp, Warning, TEXT("AI: Lost sight of player! Returning to gooning..."));
				}
			}
		}
	}
}
