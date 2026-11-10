#include "NpcSpawner.h"
#include "Npc.h"
#include "PatrolRoute.h"

// Sets default values
ANpcSpawner::ANpcSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void ANpcSpawner::BeginPlay()
{
	Super::BeginPlay();

	SpawnNPCs();
}

void ANpcSpawner::SpawnNPCs() {

	for (const FNpcSpawnData& Data : NPCsToSpawn) {

		if (!Data.NpcClass) {

			UE_LOG(LogTemp, Warning, TEXT("Spawner: An NPC slot is empty! Skipping..."));
			continue;
		}

		FVector SpawnLocation = GetActorLocation() + Data.SpawnOffset;
		FRotator SpawnRotation = GetActorRotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		ANpc* SpawnedNpc = GetWorld()->SpawnActor<ANpc>(Data.NpcClass, SpawnLocation, SpawnRotation, SpawnParams);

		if (SpawnedNpc) {

			SpawnedNpc->AssignedRoute = Data.AssignedRoute;

			UE_LOG(LogTemp, Log, TEXT("Spawner: Successfully spawned %s"), *SpawnedNpc->GetName());
		}
	}
}

