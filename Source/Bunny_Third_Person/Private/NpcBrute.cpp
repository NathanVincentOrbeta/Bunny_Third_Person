#include "NpcBrute.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CombatComponent.h"

ANpcBrute::ANpcBrute() {

	MaxHealth = 300.0f;
	Health = MaxHealth;

	PatrolSpeed = 150.0f;
	ChaseSpeed = 250.0f;
	GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
	GetCharacterMovement()->Mass = 250.0f;

	GetCapsuleComponent()->InitCapsuleSize(60.0f, 110.0f);
}

float ANpcBrute::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) {

	float ReducedDamage = DamageAmount * (1.0f - FMath::Clamp(DamageResistance, 0.0f, 1.0f));
	return Super::TakeDamage(ReducedDamage, DamageEvent, EventInstigator, DamageCauser);
}