#include "NpcSpawner.h"
#include "Npc.h"
#include "PatrolRoute.h"
#include "SpawnManager.h"
#include "TimerManager.h"

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

	TrySpawn();
}

void ANpcSpawner::TrySpawn() {

	if (!SpawnManager) {

		UE_LOG(LogTemp, Error, TEXT("Spawner: No SpawnManager assigned!"));
		return;
	}

	if (SpawnManager->RequestSpawn()) {

		// 2. If granted, don't spawn yet! Start a timer instead.
		GetWorldTimerManager().SetTimer(
			SpawnTimerHandle,
			this,
			&ANpcSpawner::PerformSpawn,
			SpawnDelay,
			false // Do not loop
		);

		UE_LOG(LogTemp, Log, TEXT("Spawner: Slot reserved. Spawning in %f seconds..."), SpawnDelay);
	}
}

void ANpcSpawner::PerformSpawn() {
	// Now we actually spawn the NPC
	if (NPCsToSpawn.Num() == 0) return;

	int32 RandomIndex = FMath::RandRange(0, NPCsToSpawn.Num() - 1);
	FNpcSpawnData Data = NPCsToSpawn[RandomIndex];

	if (!Data.NpcClass) return;

	FVector SpawnLoc = GetActorLocation() + Data.SpawnOffset;
	ANpc* SpawnedNpc = GetWorld()->SpawnActor<ANpc>(Data.NpcClass, SpawnLoc, GetActorRotation());

	if (SpawnedNpc) {
		SpawnedNpc->AssignedRoute = Data.AssignedRoute;
		SpawnedNpc->OnNpcDeath.AddDynamic(this, &ANpcSpawner::HandleNpcDeath);
		ActiveNPCs.Add(SpawnedNpc);

		UE_LOG(LogTemp, Log, TEXT("Spawner: NPC has finally arrived!"));
	}
}


void ANpcSpawner::HandleNpcDeath(ANpc* DeadNpc) {

	ActiveNPCs.Remove(DeadNpc);

	if (SpawnManager) {

		SpawnManager->NotifyDeath();
	}

	TrySpawn();
}

