#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NpcSpawner.generated.h"

class ANpc;
class APatrolRoute;
class ASpawnManager;

USTRUCT(BlueprintType)
struct FNpcSpawnData {

	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	TSubclassOf<ANpc> NpcClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	FVector SpawnOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	APatrolRoute* AssignedRoute = nullptr;
};

UCLASS()
class BUNNY_THIRD_PERSON_API ANpcSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANpcSpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Setup")
	ASpawnManager* SpawnManager;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	TArray<FNpcSpawnData> NPCsToSpawn;

	UPROPERTY(EditAnywhere, Category = "Spawner")
	float SpawnDelay = 3.0f;

	UPROPERTY()
	TArray<ANpc*> ActiveNPCs;

	void TrySpawn();

	void PerformSpawn();

	UFUNCTION()
	void HandleNpcDeath(ANpc* DeadNpc);

	FTimerHandle SpawnTimerHandle;
};
