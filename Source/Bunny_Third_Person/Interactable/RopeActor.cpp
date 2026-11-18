// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactable/RopeActor.h"

// Sets default values
ARopeActor::ARopeActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	RopeMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RopeMesh"));
	RootComponent = RopeMesh;
}

// Called when the game starts or when spawned
void ARopeActor::BeginPlay()
{
	Super::BeginPlay();

	if (RopeMesh) 
	{	
		RopeMesh->SetAllBodiesBelowSimulatePhysics(PhysicsStartBone, true, true);
	}
	
}

// Called every frame
void ARopeActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

