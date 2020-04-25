// Fill out your copyright notice in the Description page of Project Settings.


#include "GridNode.h"
#include "Food.h"
#include "Utility.h"
#include "Agent.h"

GridNode::GridNode()
{
	X = 0;
	Y = 0;

	GridType = Open;
	Parent = nullptr;
	IdleObjectAtLocation = nullptr;

	G = 0;
	H = 0;
	F = 0;
	
}

float GridNode::GetTravelCost() const
{
	switch(GridType)
	{
		case Open:
			return 1;
		case Wall:
			return 999999;
		case Forest:
			return 7;
		case Swamp:
			return 10;
		case Water:
			return 15;
		default:
			return 1;
	}
}

void GridNode::CalculateH_Dijkstra()
{
	H = GetTravelCost();
}

AFood* GridNode::GetFood()
{
	AFood* retval = nullptr;

	if (IdleObjectAtLocation != nullptr)
	{
		retval = Cast<AFood>(IdleObjectAtLocation);
	}

	return retval;
}

bool GridNode::HasFood()
{
	return GetFood() != nullptr;
}

FVector2D GridNode::GetGridNodeActorLocation()
{
	return UtilityFunctions::GridPositionToLocation(X, Y);
}

bool GridNode::IsAgentIdling()
{
	if (IdleObjectAtLocation != nullptr)
	{
		return Cast<AAgent>(IdleObjectAtLocation) != nullptr;
	}

	return false;
}

bool GridNode::IsTraversable()
{
	return GridType != GridNode::Wall;
}
