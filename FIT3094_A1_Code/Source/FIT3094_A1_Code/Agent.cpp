// Fill out your copyright notice in the Description page of Project Settings.


#include "Agent.h"

#include "LevelGenerator.h"

// Sets default values
AAgent::AAgent()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Health = 50;
	MoveSpeed = 100;
	Tolerance = 20;
}

// Called when the game starts or when spawned
void AAgent::BeginPlay()
{
	Super::BeginPlay();

	// Set a timer for every two seconds and call the decrease health function
	GetWorldTimerManager().SetTimer(TimerHandle, this, &AAgent::DecreaseHealth, 2.0f, true, 2.0f);
}

void AAgent::DecreaseHealth()
{
	// Decrease health by one and if 0 then destroy object
	Health--;

	if(Health <= 0)
	{
		GetWorldTimerManager().ClearTimer(TimerHandle);
		Destroy();
	}
}

// Called every frame
void AAgent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(Path.Num() > 0)
	{
		FVector CurrentPosition = GetActorLocation();
		
		float TargetXPos = Path[0]->X * ALevelGenerator::GRID_SIZE_WORLD;
		float TargetYPos = Path[0]->Y * ALevelGenerator::GRID_SIZE_WORLD;

		FVector TargetPosition(TargetXPos, TargetYPos, CurrentPosition.Z);

		FVector Direction = TargetPosition - CurrentPosition;
		Direction.Normalize();

		CurrentPosition += Direction * MoveSpeed * DeltaTime;

		if(FVector::Dist(CurrentPosition, TargetPosition) <= Tolerance)
		{
			CurrentPosition = TargetPosition;
			Path.RemoveAt(0);
		}
	}
}

