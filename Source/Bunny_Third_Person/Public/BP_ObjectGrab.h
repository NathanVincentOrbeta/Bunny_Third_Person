#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Containers/List.h"
#include "BP_ObjectGrab.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class ABunny_Third_PersonCharacter;

UCLASS()
class BUNNY_THIRD_PERSON_API ABP_ObjectGrab : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABP_ObjectGrab();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MyObjectBox; 

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* CollisonSphere;

	UPROPERTY()
	bool bIsHeld;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedCompnent, 
		AActor* OtherActor, UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnEndOverlap(UPrimitiveComponent* OverlappedCompnent,
		AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable, Category = "Grab")
	void GrabObject(USceneComponent* AttachToComponent, FName SocketName = NAME_None);

	UFUNCTION(BlueprintCallable, Category = "Grab")
	void DropObject();

	UFUNCTION(BlueprintCallable, Category = "Grab")
	bool IsHeld() const { return bIsHeld; }

	//Items Drop physics
	UFUNCTION(BlueprintCallable, Category = "Grab")
	void DropAndImpulse(const FVector& Impulse);

	//Test idea
	UFUNCTION(BlueprintCallable, Category = "Grab")
	void PlaceAndDrop(const FVector& TargetLocation, const FVector& Impulse);
};
