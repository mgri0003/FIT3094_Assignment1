// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelGenerator.h"

#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// Sets default values
ALevelGenerator::ALevelGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

TArray<GridNode*> ALevelGenerator::CalculateAgentPath(GridNode* startNode)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, FString::Printf(TEXT("CalculateAgentPath() Called!")));

	TArray<GridNode*> retVal;

	TArray<GridNode*> openList;
	TArray<GridNode*> closedList;
	GridNode* currentNode = nullptr;

	startNode->G = 0;
	startNode->CalculateH_Dijkstra();
	startNode->CalculateFitness(); //For Dijkstra, it will use H to calculate (but but we arent using distance to the goal as a parameter)

	openList.Add(startNode);

	//While (OpenList is not empty)
	while (openList.Num() > 0)
	{
		//currentNode = RemoveNodeWithSmallest_f from OpenList
		currentNode = RemoveNodeWithSmallestFitness(openList);
		if (currentNode)
		{
			//Add currentNode to ClosedList
			closedList.Add(currentNode);

			//Return path if the node is food!
			if (currentNode->HasFood())
			{
				//populate the return values
				GridNode* tempNode = currentNode;
				while (tempNode)
				{
					retVal.Insert(tempNode, 0);

					tempNode = tempNode->Parent;
				}

				//Clean up openlist & closedlist parents.
				for (GridNode* gn : openList)
				{
					if (gn)
					{
						gn->Parent = nullptr;
					}
				}
				for (GridNode* gn : closedList)
				{
					if (gn)
					{
						gn->Parent = nullptr;
					}
				}

				GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("CalculateAgentPath(); Path Generation Success!")));

				return retVal; //no return value, path has already been made!
			}

			//For Each (nextNode accessible from currentNode)
			TArray<GridNode*> accessibleNodes = GetAccessibleNodes(currentNode);
			for (GridNode* nextNode : accessibleNodes)
			{
				//If (nextNode is in ClosedList)
				if (closedList.Contains(nextNode))
				{
					//Skip nextNode
					continue;
				}
				else
				{
					//possible_g = currentNode.g + DistanceBetween(currentNode, nextNode)
					float possible_g = currentNode->G + CalculateDistanceBetween(currentNode, nextNode);

					//possible_g_isBetter = false
					bool possible_g_isBetter = false;

					//if(nextNode is not in OpenList)
					if (!openList.Contains(nextNode))
					{
						//Add nextNode to OpenList
						openList.Add(nextNode);

						//nextNode.h = EstimateCost (nextNode, goalNode)
						nextNode->CalculateH_Dijkstra();

						//possible_g_isBetter = true
						possible_g_isBetter = true;
					}
					//Else If (possible_g < nextNode.g)
					else if (possible_g < nextNode->G)
					{
						//possible_g_isBetter = true
						possible_g_isBetter = true;
					}

					//If (possible_g_isBetter is true)
					if (possible_g_isBetter)
					{
						////nextNode.cameFrom = currentNode
						nextNode->Parent = currentNode;

						//nextNode.g = possible_g
						nextNode->G = possible_g;

						//nextNode.f = nextNode.g + nextNode.h
						nextNode->CalculateFitness(); //For Dijkstra, it will use H to calculate (but since its always 0 it shouldnt matter)
					}

				}
			}
		}
		else
		{
			//somethign went very wrong, the node wasnt taken out of the openlist or the node that WAS taken out is null. but the openlist still has nodes!?!?
			//break out so we dont get infinitely stuck
			break;
		}
	}

	//Clean up openlist & closedlist parents.
	for (GridNode* gn : openList)
	{
		if (gn)
		{
			gn->Parent = nullptr;
		}
	}
	for (GridNode* gn : closedList)
	{
		if (gn)
		{
			gn->Parent = nullptr;
		}
	}

	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("CalculateAgentPath(); Path Generation Failed!")));
	return retVal;
	//Return null --- failed to find a path!
}

// Called when the game starts or when spawned
void ALevelGenerator::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ALevelGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//DEBUG
	for (AAgent* agent : AgentActors)
	{
		if (agent)
		{
			FVector2D agentGridPos = agent->GetActorPositionAsGridPosition();
			GEngine->AddOnScreenDebugMessage(-1, -1, FColor::White, FString::Printf(TEXT("Agent Health: %d | GridPos: %d, %d"), agent->Health, (int)agentGridPos.X, (int)agentGridPos.Y) );
		}
	}

	// Should spawn more food if there are not the right number
	//when uneated food runs out
	if (UneatenFoodActors.Num() == 0)
	{
		//spawn more food!
		SpawnFood();
	}
}

void ALevelGenerator::GenerateWorldFromFile(TArray<FString> WorldArrayStrings)
{
	// If empty array exit immediately something is horribly wrong
	if(WorldArrayStrings.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("World Array is empty!"));
		return;
	}

	// Second line is Height (aka X value)
	FString Height = WorldArrayStrings[1];
	Height.RemoveFromStart("height ");
	MapSizeX = FCString::Atoi(*Height);
	UE_LOG(LogTemp, Warning, TEXT("Height: %d"), MapSizeX);

	// Third line is Width (aka Y value)
	FString Width = WorldArrayStrings[2];
	Width.RemoveFromStart("width ");
	MapSizeY = FCString::Atoi(*Width);
	UE_LOG(LogTemp, Warning, TEXT("Width: %d"), MapSizeY);
	
	// After removing top 4 lines this is the map itself so iterate each line
	for (int LineNum = 4; LineNum < WorldArrayStrings.Num(); LineNum++)
	{
		for (int CharNum = 0; CharNum < WorldArrayStrings[LineNum].Len(); CharNum++)
		{
			CharMapArray[LineNum-4][CharNum] = WorldArrayStrings[LineNum][CharNum];
		}
	}

	GenerateNodeGrid();
	SpawnWorldActors();
}

void ALevelGenerator::SpawnWorldActors()
{
	UWorld* World = GetWorld();

	// Make sure that all blueprints are connected. If not then fail
	if(WallBlueprint && OpenBlueprint && WaterBlueprint && SwampBlueprint && TreeBlueprint)
	{
		// For each grid space spawn an actor of the correct type in the game world
		for(int x = 0; x < MapSizeX; x++)
		{
			for (int y = 0; y < MapSizeY; y++)
			{
				float XPos = x * GRID_SIZE_WORLD;
				float YPos = y * GRID_SIZE_WORLD;

				FVector Position(XPos, YPos, 0);

				switch (CharMapArray[x][y])
				{
					case '.':
					case 'G':
						World->SpawnActor(OpenBlueprint, &Position, &FRotator::ZeroRotator);
						break;
					case '@':
					case 'O':
						World->SpawnActor(WallBlueprint, &Position, &FRotator::ZeroRotator);
						break;
					case 'T':
						World->SpawnActor(TreeBlueprint, &Position, &FRotator::ZeroRotator);
						break;
					case 'S':
						World->SpawnActor(SwampBlueprint, &Position, &FRotator::ZeroRotator);
						break;
					case 'W':
						World->SpawnActor(WaterBlueprint, &Position, &FRotator::ZeroRotator);
						break;
					default:
						break;
				}
			}
		}
	}

	// Generate Initial Agent Positions
	SpawnAgents();

	// Generate Initial Food Positions
	SpawnFood();
}

// Generates the grid of nodes used for pathfinding and also for placement of objects in the game world
void ALevelGenerator::GenerateNodeGrid()
{
	for(int X = 0; X < MapSizeX; X++)
	{
		for(int Y = 0; Y < MapSizeY; Y++)
		{
			WorldArray[X][Y] = new GridNode();
			WorldArray[X][Y]->X = X;
			WorldArray[X][Y]->Y = Y;

			// Characters as defined from the map file
			switch(CharMapArray[X][Y])
			{
				case '.':
				case 'G':
					WorldArray[X][Y]->GridType = GridNode::Open;
					break;
				case '@':
				case 'O':
					WorldArray[X][Y]->GridType = GridNode::Wall;
					break;
				case 'T':
					WorldArray[X][Y]->GridType = GridNode::Forest;
					break;
				case 'S':
					WorldArray[X][Y]->GridType = GridNode::Swamp;
					break;
				case 'W':
					WorldArray[X][Y]->GridType = GridNode::Water;
					break;
			}
		}
	}
}

// Reset all node values (F, G, H & Parent)
void ALevelGenerator::ResetAllNodes()
{
	for (int X = 0; X < MapSizeX; X++)
	{
		for (int Y = 0; Y < MapSizeY; Y++)
		{
			WorldArray[X][Y]->F = 0;
			WorldArray[X][Y]->G = 0;
			WorldArray[X][Y]->H = 0;
			WorldArray[X][Y]->Parent = nullptr;
		}
	}
}

float ALevelGenerator::CalculateDistanceBetween(GridNode* first, GridNode* second)
{
	FVector distToTarget = FVector(second->X - first->X,
		second->Y - first->Y, 0);
	return distToTarget.Size();
}

GridNode* ALevelGenerator::RemoveNodeWithSmallestFitness(TArray<GridNode*>& openList)
{
	GridNode* retVal = nullptr;

	if (openList.Num() > 0 && openList[0])
	{
		//assume the lowest fitness node is the first one in the list
		int selectedIdx = 0;
		float lowestFitness = openList[0]->F;
		retVal = openList[0];

		//go through each node and see if there are any lower ones
		for (int i = 0; i < openList.Num(); ++i)
		{
			GridNode* gn = openList[i];

			if (gn)
			{
				//found a lower fitness!
				if (gn->F < lowestFitness)
				{
					lowestFitness = gn->F;
					selectedIdx = i;
				}
			}
		}

		if (selectedIdx != -1)
		{
			retVal = openList[selectedIdx];
			openList.RemoveAt(selectedIdx);
		}
	}

	return retVal;
}

TArray<GridNode*> ALevelGenerator::GetAccessibleNodes(GridNode* currentNode)
{
	TArray<GridNode*> retVal;
	//GridNode* tempNode = nullptr;

	TArray<GridNode*> nodesToProcess;

	//LEFT
	nodesToProcess.Add(GetGridNodeFromWorldArray(currentNode->X, currentNode->Y - 1));
	//TOP
	nodesToProcess.Add(GetGridNodeFromWorldArray(currentNode->X + 1, currentNode->Y));
	//RIGHT
	nodesToProcess.Add(GetGridNodeFromWorldArray(currentNode->X, currentNode->Y + 1));
	//BOTTOM
	nodesToProcess.Add(GetGridNodeFromWorldArray(currentNode->X - 1, currentNode->Y));

	for (GridNode* gn : nodesToProcess)
	{
		if (gn)
		{
			bool canBeAccessed = gn->GridType != GridNode::Wall;

			if (canBeAccessed)
			{
				retVal.Add(gn);
			}
		}
	}

	return retVal;
}

void ALevelGenerator::SpawnAgents()
{
	if (AgentBlueprint)
	{
		for (int i = 0; i < NUM_AGENTS; i++)
		{
			int RandXPos = 0;
			int RandYPos = 0;
			bool isFree = false;

			while (!isFree) {
				RandXPos = FMath::RandRange(0, MapSizeX - 1);
				RandYPos = FMath::RandRange(0, MapSizeY - 1);

				if (WorldArray[RandXPos][RandYPos]->GridType == GridNode::Open && WorldArray[RandXPos][RandYPos]->ObjectAtLocation == nullptr)
				{
					isFree = true;
				}
			}

			FVector Position(RandXPos * GRID_SIZE_WORLD, RandYPos * GRID_SIZE_WORLD, 20);
			AAgent* Agent = GetWorld()->SpawnActor<AAgent>(AgentBlueprint, Position, FRotator::ZeroRotator);

			WorldArray[RandXPos][RandYPos]->ObjectAtLocation = Agent;


			Agent->SetLevelGenerator(this);
			AgentActors.Add(Agent);
		}
	}
}

void ALevelGenerator::SpawnFood()
{
	if (FoodBlueprint)
	{
		for (int i = 0; i < NUM_FOOD; i++)
		{
			int RandXPos = 0;
			int RandYPos = 0;
			bool isFree = false;

			while (!isFree) {
				RandXPos = FMath::RandRange(0, MapSizeX - 1);
				RandYPos = FMath::RandRange(0, MapSizeY - 1);

				if (WorldArray[RandXPos][RandYPos]->GridType == GridNode::Open && WorldArray[RandXPos][RandYPos]->ObjectAtLocation == nullptr)
				{
					isFree = true;
				}
			}

			FVector Position(RandXPos * GRID_SIZE_WORLD, RandYPos * GRID_SIZE_WORLD, 20);
			AFood* NewFood = GetWorld()->SpawnActor<AFood>(FoodBlueprint, Position, FRotator::ZeroRotator);

			WorldArray[RandXPos][RandYPos]->ObjectAtLocation = NewFood;
			UneatenFoodActors.Add(NewFood);
		}
	}

	//let all the agents know fodo has been spawned
	Event_OnFoodSpawned();
}

void ALevelGenerator::Event_NotifyAllAgentsToRecalculatePaths()
{
	for (AAgent* agent : AgentActors)
	{
		if (agent)
		{
			if (!agent->HasPath())
			{
				agent->RecalculatePathToFood();
			}
		}
	}
}

void ALevelGenerator::Event_OnFoodSpawned()
{
	Event_NotifyAllAgentsToRecalculatePaths();
}

void ALevelGenerator::Event_OnFoodEaten(AFood* eatenFood)
{
	if (eatenFood)
	{
		//remove eaten food from eaten array
		UneatenFoodActors.Remove(eatenFood);

		//add eaten food to eaten array
		EatenFoodActors.Add(eatenFood);

		//clean up eaten food actors
		while (EatenFoodActors.Num() > 0)
		{
			FVector foodNodeLocation = EatenFoodActors[0]->GetActorLocation();
			GridNode* foodNode = GetGridNodeFromWorldArray(UtilityFunctions::LocationToGridPosition(foodNodeLocation.X, foodNodeLocation.Y));
			if (foodNode && foodNode->HasFood())
			{
				foodNode->ObjectAtLocation = nullptr;
			}

			EatenFoodActors[0]->Destroy();
			EatenFoodActors.RemoveAt(0);
		}

		//let all agents know food has been eaten, so recalculate paths!
		Event_NotifyAllAgentsToRecalculatePaths();
	}
}

GridNode* ALevelGenerator::GetGridNodeFromWorldArray(int x, int y)
{
	GridNode* retVal = nullptr;

	//check bounds
	if ( (x >= 0 && x < MapSizeX) && (y >= 0 && y < MapSizeY) )
	{
		retVal = WorldArray[x][y];
	}

	return retVal;
}

GridNode* ALevelGenerator::GetGridNodeFromWorldArray(FVector2D gridPos)
{
	return GetGridNodeFromWorldArray(gridPos.X, gridPos.Y);
}
