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

			// 1. Create the array to hold the results
			TArray<AActor*> CurrentlyPerceivedActors;

			// 2. CALL THE FUNCTION WITH BOTH ARGUMENTS
			// Argument 1: UAISense_Sight::StaticClass() -> Tells it to only look for things seen
			// Argument 2: CurrentlyPerceivedActors -> The array to fill with results
			PerceptionComp->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), CurrentlyPerceivedActors);

			// 3. Check if the actor that triggered the update is still in the "seen" list
			if (CurrentlyPerceivedActors.Contains(Actor)) {

				// PLAYER IS CURRENTLY SEEN
				if (Blackboard) {
					Blackboard->SetValueAsObject(TEXT("TargetActor"), Actor);
					UE_LOG(LogTemp, Warning, TEXT("AI: Player spotted via Sight!"));
				}
			}
			else {

				// PLAYER IS NO LONGER SEEN
				if (Blackboard) {
					Blackboard->ClearValue(TEXT("TargetActor"));
					UE_LOG(LogTemp, Warning, TEXT("AI: Lost sight of player! Returning to patrol."));
				}
			}
		}
	}
}
