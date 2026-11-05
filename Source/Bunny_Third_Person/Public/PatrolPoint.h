#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PatrolPoint.generated.h"

UCLASS()
class BUNNY_THIRD_PERSON_API APatrolPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	APatrolPoint() {
		PrimaryActorTick.bCanEverTick = false;
		// Add a simple visual so you can see it in the editor
		RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	}

};
