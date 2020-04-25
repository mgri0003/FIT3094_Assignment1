// Fill out your copyright notice in the Description page of Project Settings.


#include "GridNode.h"
#include "Food.h"
#include "Utility.h"

GridNode::GridNode()
{
	X = 0;
	Y = 0;

	GridType = Open;
	Parent = nullptr;
	ObjectAtLocation = nullptr;

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

	if (ObjectAtLocation != nullptr)
	{
		retval = Cast<AFood>(ObjectAtLocation);
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
