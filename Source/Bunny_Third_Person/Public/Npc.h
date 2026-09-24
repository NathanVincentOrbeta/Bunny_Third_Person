#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Npc.generated.h"

UCLASS()
class BUNNY_THIRD_PERSON_API ANpc : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ANpc();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Default Values 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	float Health = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	UAnimMontage* DeathMontage;

	//Take damage handler
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	// Nathan must die
	void Die();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Nathan - Attaching Combat Component here to use for attack mechanic
	UPROPERTY(VisibleAnywhere)
	class UCombatComponent* NPCCombatComp;


};
