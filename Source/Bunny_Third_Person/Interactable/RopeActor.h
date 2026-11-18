// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RopeActor.generated.h"

UCLASS()
class BUNNY_THIRD_PERSON_API ARopeActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARopeActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rope");
	USkeletalMeshComponent* RopeMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Rope");
	FName PhysicsStartBone = TEXT("Bone_120");

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
