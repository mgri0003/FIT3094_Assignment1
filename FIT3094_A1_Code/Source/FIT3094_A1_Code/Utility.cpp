// Fill out your copyright notice in the Description page of Project Settings.

#include "Utility.h"


FVector2D UtilityFunctions::GridPositionToLocation(FVector2D gridPosition)
{
	return gridPosition * GRID_SIZE_WORLD;
}

FVector2D UtilityFunctions::GridPositionToLocation(int gridPosX, int gridPosY)
{
	return GridPositionToLocation(FVector2D(gridPosX, gridPosY));
}

FVector2D UtilityFunctions::LocationToGridPosition(FVector2D location)
{
	int gridPosX = FMath::RoundToInt(location.X / GRID_SIZE_WORLD);
	int gridPosY = FMath::RoundToInt(location.Y / GRID_SIZE_WORLD);
	return FVector2D(gridPosX, gridPosY);
}

FVector2D UtilityFunctions::LocationToGridPosition(int locationX, int locationY)
{
	return LocationToGridPosition(FVector2D(locationX, locationY));
}
