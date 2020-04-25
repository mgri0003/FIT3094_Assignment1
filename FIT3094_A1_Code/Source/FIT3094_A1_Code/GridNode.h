// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

class AAgent;
class AFood;

/**
 * 
 */
class FIT3094_A1_CODE_API GridNode
{

public:

	// Types of grid nodes
	enum GRID_TYPE
	{
		Open,
		Wall,
		Forest,
		Swamp,
		Water
	};

	GridNode();

	float GetTravelCost() const;

	// Position in Grid
	int X;
	int Y;

	// Informed Search Variables
	int G;
	float H;
	float F;

	// Type of grid space
	GRID_TYPE GridType;
	
	// Pointer to previous Node (Only used in searching)
	GridNode* Parent;

	// Object at current location
	AActor* IdleObjectAtLocation;

	void CalculateFitness()
	{
		F = (float)G + H;
	}

	void CalculateH_Dijkstra();

	AFood* GetFood();
	bool HasFood();

	FVector2D GetGridNodeActorLocation();

	bool IsAgentIdling();

	bool IsTraversable();

	void SetAgentUsing(AAgent* newAgentUsing);
	bool IsAgentUsing();

private:
	AAgent* m_agentUsing = nullptr;

};
