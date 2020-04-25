// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridNode.h"
#include "GameFramework/Actor.h"
#include "Agent.generated.h"

class ALevelGenerator;



#define AGENT_MAX_HEALTH (50)
#define AGENT_SPEED (100)
#define AGENT_TOLERANCE (20)



UCLASS()
class FIT3094_A1_CODE_API AAgent : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAgent();

	int Health;
	float MoveSpeed;
	float Tolerance;

	void SetLevelGenerator(ALevelGenerator* newLG) { m_levelGenerator = newLG; }
	ALevelGenerator* GetLevelGenerator() { return m_levelGenerator; }

	void RecalculatePathToFood();

	FVector2D GetActorPositionAsGridPosition();

	GridNode* GetNodeOnPath(int idx);
	bool HasPath() { return m_currentPath.Num() > 0; }
	void ResetPath();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Timed function that decreases health every 2 seconds;
	void DecreaseHealth();

	// Handle for Timer
	FTimerHandle TimerHandle;

	void OnReachedNode(GridNode* reachedNode);
	void OnDeath();

	void AttemptEatFoodAtNode(GridNode* node);
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	TArray<GridNode*> m_currentPath;
	ALevelGenerator* m_levelGenerator = nullptr;
};
