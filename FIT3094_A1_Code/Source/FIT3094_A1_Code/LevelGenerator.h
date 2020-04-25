// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Agent.h"
#include "CoreMinimal.h"
#include "Food.h"
#include "GameFramework/Actor.h"
#include "GridNode.h"
#include "Utility.h"
#include "LevelGenerator.generated.h"

UCLASS()
class FIT3094_A1_CODE_API ALevelGenerator : public AActor
{
	GENERATED_BODY()

public:

	static const int NUM_FOOD = 25;
	static const int NUM_AGENTS = 5;
	
	// Sets default values for this actor's properties
	ALevelGenerator();

	UPROPERTY(BlueprintReadOnly)
		int MapSizeX;
	UPROPERTY(BlueprintReadOnly)
		int MapSizeY;
	
	// This is a 2D Array for holding all the elements of the world loaded from file
	char CharMapArray[MAX_MAP_SIZE][MAX_MAP_SIZE];

	// This is a 2D Array for holding nodes for each part of the world
	GridNode* WorldArray[MAX_MAP_SIZE][MAX_MAP_SIZE];

	UPROPERTY()
		TArray<AFood*> UneatenFoodActors;
	UPROPERTY()
		TArray<AFood*> EatenFoodActors;

	UPROPERTY()
		TArray<AAgent*> AgentActors;

	// Actors for spawning into the world
	UPROPERTY(EditAnywhere, Category = "Entities")
		TSubclassOf<AActor> WallBlueprint;
	UPROPERTY(EditAnywhere, Category = "Entities")
		TSubclassOf<AActor> OpenBlueprint;
	UPROPERTY(EditAnywhere, Category = "Entities")
		TSubclassOf<AActor> TreeBlueprint;
	UPROPERTY(EditAnywhere, Category = "Entities")
		TSubclassOf<AActor> SwampBlueprint;
	UPROPERTY(EditAnywhere, Category = "Entities")
		TSubclassOf<AActor> WaterBlueprint;
	UPROPERTY(EditAnywhere, Category = "Entities")
		TSubclassOf<AActor> FoodBlueprint;
	UPROPERTY(EditAnywhere, Category = "Entities")
		TSubclassOf<AActor> Food2Blueprint;
	UPROPERTY(EditAnywhere, Category = "Entities")
		TSubclassOf<AActor> AgentBlueprint;
	UPROPERTY(EditAnywhere, Category = "Entities")
		TSubclassOf<AActor> Agent2Blueprint;

	TArray<GridNode*> CalculateAgentPath(GridNode* startNode, const EFoodType targetFoodType);


	GridNode* GetGridNodeFromWorldArray(int x, int y);
	GridNode* GetGridNodeFromWorldArray(FVector2D gridPos);

	//EVENTS
	void Event_NotifyAllAgentsToRecalculatePaths();
	void Event_OnFoodSpawned();
	void Event_OnAgentDeath(AAgent* deadAgent);
	void Event_OnFoodEaten(AFood* eatenFood);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void SpawnWorldActors();

	void GenerateNodeGrid();
	void ResetAllNodes();

	float CalculateDistanceBetween(GridNode* first, GridNode* second);
	GridNode* RemoveNodeWithSmallestFitness(TArray<GridNode*>& openList);
	TArray<GridNode*> GetAccessibleNodes(GridNode* currentNode, GridNode* startNode, const EFoodType targetFoodType);
	bool IsNodeAccessible(GridNode* node, GridNode* startNode, const EFoodType targetFoodType);

	void SpawnAgents();
	void SpawnFood();



public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
		void GenerateWorldFromFile(TArray<FString> WorldArray);

};
