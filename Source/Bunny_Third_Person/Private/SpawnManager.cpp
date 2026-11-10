#include "SpawnManager.h"

ASpawnManager::ASpawnManager()
{
	PrimaryActorTick.bCanEverTick = false;
	CurrentNpcCount = 0;
}

bool ASpawnManager::RequestSpawn()
{
	if (CurrentNpcCount < MaxGlobalNPCs)
	{
		CurrentNpcCount++;
		return true;
	}

	return false;
}

void ASpawnManager::NotifyDeath()
{
	// Ensure we never go below 0
	CurrentNpcCount = FMath::Max(0, CurrentNpcCount - 1);
}
