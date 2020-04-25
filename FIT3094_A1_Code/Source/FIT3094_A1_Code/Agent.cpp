// Fill out your copyright notice in the Description page of Project Settings.


#include "Agent.h"

#include "LevelGenerator.h"
#include "GameFramework/Actor.h"
#include "Utility.h"
#include "Runtime\Engine\Public\TimerManager.h"
#include "CoreMinimal.h"


// Sets default values
AAgent::AAgent()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Health = AGENT_MAX_HEALTH;
	MoveSpeed = AGENT_SPEED;
	Tolerance = AGENT_TOLERANCE;
}

void AAgent::RecalculatePathToFood()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, FString::Printf(TEXT("RecalculatePathToFood() Called!")));

	ResetPath();

	if (GetLevelGenerator())
	{
		GridNode* startingNode = GetLevelGenerator()->GetGridNodeFromWorldArray(GetActorPositionAsGridPosition());
		m_currentPath = GetLevelGenerator()->CalculateAgentPath(startingNode);

		if (HasPath())
		{
			//if we have path, we are not longer at this location
			if (startingNode->IdleObjectAtLocation == this)
			{
				startingNode->IdleObjectAtLocation = nullptr;
			}
		}
		else
		{
			//we dont haev a path, we are still chilling here..
			startingNode->IdleObjectAtLocation = this;
		}
	}
}

FVector2D AAgent::GetActorPositionAsGridPosition()
{
	FVector2D actorPos2D;
	actorPos2D.X = GetActorLocation().X;
	actorPos2D.Y = GetActorLocation().Y;
	return UtilityFunctions::LocationToGridPosition(actorPos2D);
}

GridNode* AAgent::GetNodeOnPath(int idx)
{
	GridNode* retVal = nullptr;

	if (idx >= 0 && idx < m_currentPath.Num())
	{
		retVal = m_currentPath[idx];
	}

	return retVal;
}

void AAgent::ResetPath()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, FString::Printf(TEXT("Agent's ResetPath() Called!")));
	m_currentPath.Empty();
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
		OnDeath();
	}
}

void AAgent::OnReachedNode(GridNode* reachedNode)
{
	if (reachedNode->HasFood())
	{
		AttemptEatFoodAtNode(reachedNode);
	}
}

void AAgent::OnDeath()
{
	GetLevelGenerator()->Event_OnAgentDeath(this);

	GetWorldTimerManager().ClearTimer(TimerHandle);
	Destroy();
}

void AAgent::AttemptEatFoodAtNode(GridNode* node)
{
	if (node)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("AAgent::AttemptEatFoodAtNode; Agent Successfully Ate Food!")));

		//food eaten!!!
		GetLevelGenerator()->Event_OnFoodEaten(node->GetFood());

		//reset health
		Health = AGENT_MAX_HEALTH;
	}
}

// Called every frame
void AAgent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//DEBUG
	for (GridNode* gn : m_currentPath)
	{
		if (gn)
		{
			FVector2D debugLocation2D = gn->GetGridNodeActorLocation();
			DrawDebugSphere(GetWorld(), FVector(debugLocation2D.X, debugLocation2D.Y,0) , 50.0f, 5, FColor::Magenta, false, -1.0f, 0, 2.0f);
		}
	}


	if(m_currentPath.Num() > 0)
	{
		GridNode* targetNode = GetNodeOnPath(0);

		FVector currentPosition = GetActorLocation();
		
		FVector2D targetPosition2D = UtilityFunctions::GridPositionToLocation(targetNode->X, targetNode->Y);
		FVector targetPosition(targetPosition2D.X, targetPosition2D.Y, currentPosition.Z);

		FVector Direction = targetPosition - currentPosition;
		Direction.Normalize();

		currentPosition += Direction * MoveSpeed * DeltaTime;

		SetActorLocation(currentPosition);

		if(FVector::Dist(currentPosition, targetPosition) <= Tolerance)
		{
			currentPosition = targetPosition;
			SetActorLocation(currentPosition);
			m_currentPath.RemoveAt(0);

			OnReachedNode(targetNode);
		}
	}
}