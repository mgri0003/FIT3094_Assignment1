// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DrawDebugHelpers.h"

#define MAX_MAP_SIZE (256)
#define GRID_SIZE_WORLD (100) // Grid Size in World Units

static class FIT3094_A1_CODE_API UtilityFunctions
{
public:
	static FVector2D GridPositionToLocation(FVector2D gridPosition);
	static FVector2D GridPositionToLocation(int gridPosX, int gridPosY);
	static FVector2D LocationToGridPosition(FVector2D location);
	static FVector2D LocationToGridPosition(int locationX, int locationY);
};


