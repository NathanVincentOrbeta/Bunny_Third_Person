// Fill out your copyright notice in the Description page of Project Settings.


#include "RangedCombatComponent.h"

// Sets default values
ARangedCombatComponent::ARangedCombatComponent()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ARangedCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARangedCombatComponent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

