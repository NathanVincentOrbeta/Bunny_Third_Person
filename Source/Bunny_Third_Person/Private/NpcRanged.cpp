#include "NpcRanged.h"
#include "RangedCombatComponent.h"

ANpcRanged::ANpcRanged() {

	NPCCombatComp = CreateDefaultSubobject<URangedCombatComponent>(TEXT("RangedCombatComp"));

	ChaseSpeed = 350.0f;
}

