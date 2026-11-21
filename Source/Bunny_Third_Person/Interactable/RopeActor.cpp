// Fill out your copyright notice in the Description page of Project Settings.


#include "Interactable/RopeActor.h"

// Sets default values
ARopeActor::ARopeActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* AnchorComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RopeAnchor"));
	RootComponent = AnchorComponent;

	RopeMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RopeMesh"));
	RopeMesh->SetupAttachment(RootComponent);

	RopeMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	RopeMesh->SetCollisionObjectType(ECC_PhysicsBody);
	RopeMesh->SetCollisionResponseToAllChannels(ECR_Block);
	RopeMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	RopeMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	RopeMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
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

