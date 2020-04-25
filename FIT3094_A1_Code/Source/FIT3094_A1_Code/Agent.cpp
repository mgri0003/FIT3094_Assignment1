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
#if ENABLE_DEBUG_MESSAGES
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, FString::Printf(TEXT("RecalculatePathToFood() Called!")));
#endif

	ResetCurrentPath();

	if (GetLevelGenerator())
	{
		GridNode* startingNode = GetLevelGenerator()->GetGridNodeFromWorldArray(GetActorPositionAsGridPosition());
		m_currentPath = GetLevelGenerator()->CalculateAgentPath(startingNode, GetFoodTypeCanEat());

		if (HasCurrentPath())
		{
			//keep track of agent (if we steal their path)
			AAgent* robbedAgent = nullptr;

			//if another agent was using the last node in this path...
			GridNode* lastGridNode = GetLastGridNodeOnCurrentPath();
			if (lastGridNode)
			{
				//(then we just stole another agent's food path)
				if (lastGridNode->IsAgentUsing() && lastGridNode->GetAgentUsing() != this)
				{
					robbedAgent = lastGridNode->GetAgentUsing();
				}
			}

			//the entire path's grid nodes are now in use by this agent!
			for (GridNode* gn : m_currentPath)
			{
				gn->SetAgentUsing(this);
			}

			if (robbedAgent)
			{
				//tell the robbed agent to recalc their path since it was burgled...
				robbedAgent->RecalculatePathToFood();
			}

			//if we have path, we are not longer at this location
			if (startingNode->IdleObjectAtLocation == this)
			{
				startingNode->IdleObjectAtLocation = nullptr;
			}
		}
		else
		{
			//we don't have a path, we are still chilling here..
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

GridNode* AAgent::GetLastGridNodeOnCurrentPath()
{
	return GetGridNodeOnCurrentPath(GetCurrentPathCount() - 1);
}

GridNode* AAgent::GetGridNodeOnCurrentPath(int idx)
{
	GridNode* retVal = nullptr;

	if (idx >= 0 && idx < m_currentPath.Num())
	{
		retVal = m_currentPath[idx];
	}

	return retVal;
}

void AAgent::ResetCurrentPath()
{
#if ENABLE_DEBUG_MESSAGES
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, FString::Printf(TEXT("Agent's ResetPath() Called!")));
#endif

	//the entire path's grid nodes are no longer in use by this agent!
	while (GetCurrentPathCount() > 0)
	{
		RemoveCurrentPathAt(0);
	}
}

void AAgent::RemoveCurrentPathAt(int idx)
{
	GridNode* gn = GetGridNodeOnCurrentPath(idx);
	if (gn)
	{
		//only wipe the agent using if we were the one to use it!
		if (gn->GetAgentUsing() == this)
		{
			//this grid nodes is no longer in use by this agent!
			gn->SetAgentUsing(nullptr);
		}

		m_currentPath.RemoveAt(idx);
	}
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
	
	//if a path is still present, check if there is still food at the end of the path...
	if (HasCurrentPath())
	{
		ensureAlwaysMsgf(GetLastGridNodeOnCurrentPath()->HasFood(), TEXT("An agent has stolen our food but didnt make us recalculate!?!?"));
		if (!GetLastGridNodeOnCurrentPath()->HasFood())
		{
			//should recalc path
			RecalculatePathToFood();
		}
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
#if ENABLE_DEBUG_MESSAGES
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("AAgent::AttemptEatFoodAtNode; Agent Successfully Ate Food!")));
#endif
		//food eaten!!!
		GetLevelGenerator()->Event_OnFoodEaten(node->GetFood());

		//reset health
		Health = AGENT_MAX_HEALTH;
	}
}

EFoodType AAgent::GetFoodTypeCanEat()
{
	switch (m_agentType)
	{
		case EAgentType::CARNIVORE:
		{
			return EFoodType::MEAT;
		}
		break;

		case EAgentType::VEGETARIAN:
		{
			return EFoodType::VEGETABLE;
		}
		break;
	}
	
	return EFoodType::MAX_COUNT;
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
		GridNode* targetNode = GetGridNodeOnCurrentPath(0);

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
			RemoveCurrentPathAt(0);

			OnReachedNode(targetNode);
		}
	}
}