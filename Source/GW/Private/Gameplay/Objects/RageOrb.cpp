// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Objects/RageOrb.h"

// Sets default values
ARageOrb::ARageOrb()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ARageOrb::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ARageOrb::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

