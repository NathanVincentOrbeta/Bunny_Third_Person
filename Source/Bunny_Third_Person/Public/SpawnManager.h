#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnManager.generated.h"

UCLASS()
class BUNNY_THIRD_PERSON_API ASpawnManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpawnManager();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Limits")
	int32 MaxGlobalNPCs = 4;

	UFUNCTION(BlueprintCallable, Category = "Limits")
	bool RequestSpawn();

	UFUNCTION(BlueprintCallable, Category = "Limits")
	void NotifyDeath();

private:	
	
	int32 CurrentNpcCount = 0;
};
