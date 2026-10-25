#pragma once

#include "CoreMinimal.h"
#include "Npc.h"
#include "NpcBrute.generated.h"

/**
 * 
 */
UCLASS()
class BUNNY_THIRD_PERSON_API ANpcBrute : public ANpc
{
	GENERATED_BODY()

public:
	ANpcBrute();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC | Brute")
	float DamageResistance = 0.35f;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	
};
