// Fill out your copyright notice in the Description page of Project Settings.


#include "Constants.h"
#include "Math/UnrealMathUtility.h"

Constants::Constants()
{
	this->FRAMERATE = 10.0;
	this->WALK_WAIT_THRESHOLD = 30.0;

	this->STATIONS.Add(1, FVector2D(1580, 8683));
	this->STATIONS.Add(2, FVector2D(1605, 5800));
	this->STATIONS.Add(3, FVector2D(5812, 8683));
	this->STATIONS.Add(4, FVector2D(5800, 5786));
	this->STATIONS.Add(5, FVector2D(7632, 8683));
	this->STATIONS.Add(6, FVector2D(7639, 5786));
	this->STATIONS.Add(7, FVector2D(13252, 8683));
	this->STATIONS.Add(8, FVector2D(13319, 5796));

	this->AT_STATION_THRESHOLD = 300.;
	this->NEAR_STATION_THRESHOLD = 400.;
	this->MARGIN_NEAR_SIDEWALKS = 100.;
	this->STATION_LENGTH = 500.;
	this->COLLISION_THRESHOLD = 50;

	this->GAZING_ANGLE_THRESHOLD = 40.0;
	this->GAZING_ANGLE_THRESHOLD_RADIANS = PI / 180 * this->GAZING_ANGLE_THRESHOLD;
	this->GAZING_ANGLE_THRESHOLD_COS = std::cos(this->GAZING_ANGLE_THRESHOLD_RADIANS);

	// Threshold to determine whether the user is looking at the AGV or not
	this->LOOKING_AT_AGV_THRESHOLD = 0.5;

	this->SIDEWALK_1.Add("High", 8400.);
	this->SIDEWALK_1.Add("Low", 8150.);

	this->SIDEWALK_2.Add("High", 6295.);
	this->SIDEWALK_2.Add("Low", 6045.);
}

Constants::~Constants()
{
}
