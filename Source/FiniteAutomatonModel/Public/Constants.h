// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class FINITEAUTOMATONMODEL_API Constants
{
public:
	Constants();
	~Constants();

	float FRAMERATE;
	float WALK_WAIT_THRESHOLD;
	TMap<int, FVector2D> STATIONS;
	float AT_STATION_THRESHOLD;
	float NEAR_STATION_THRESHOLD;
	float MARGIN_NEAR_SIDEWALKS;
	float STATION_LENGTH;
	float GAZING_ANGLE_THRESHOLD;
	float GAZING_ANGLE_THRESHOLD_RADIANS;
	float GAZING_ANGLE_THRESHOLD_COS;

	TMap<FString, float> SIDEWALK_1;
	TMap<FString, float> SIDEWALK_2;
};
