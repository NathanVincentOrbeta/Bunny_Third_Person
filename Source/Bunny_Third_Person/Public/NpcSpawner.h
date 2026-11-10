#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NpcSpawner.generated.h"

class ANpc;
class APatrolRoute;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawner")
	TArray<FNpcSpawnData> NPCsToSpawn;

	void SpawnNPCs();
};
