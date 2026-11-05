#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "PatrolPoint.h"
#include "PatrolRoute.generated.h"

UCLASS()
class BUNNY_THIRD_PERSON_API APatrolRoute : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APatrolRoute();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
	TArray<APatrolPoint*> Waypoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
	bool bPingPong = true;
};
