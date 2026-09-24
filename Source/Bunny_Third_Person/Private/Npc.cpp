#include "Npc.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CombatComponent.h"
#include "Components/CapsuleComponent.h"

// Sets default values
ANpc::ANpc()
{
	// Initialize Combat Component
	NPCCombatComp = CreateDefaultSubobject<UCombatComponent>(TEXT("NPCCombatComp"));

	GetCharacterMovement()->MaxWalkSpeed = 300.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 1000.0f;
}

// Called when the game starts or when spawned
void ANpc::BeginPlay()
{
	Super::BeginPlay();
	
}

void ANpc::Tick(float DeltaTime) {

	Super::Tick(DeltaTime);
}

void ANpc::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {

	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

float ANpc::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) {

	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	Health -= ActualDamage;
	UE_LOG(LogTemp, Warning, TEXT("NPC took damage! Current Health: %f"), Health);

	if (Health <= 0.0f) {

		Die();
	}
	return ActualDamage;
}

void ANpc::Die() {

	UE_LOG(LogTemp, Error, TEXT("NPC has died!"));

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->DisableMovement();

	PlayAnimMontage(DeathMontage);

	SetLifeSpan(0.3f);
}

