// Fill out your copyright notice in the Description page of Project Settings.


#include "Features.h"

// Sets default values
AFeatures::AFeatures()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	// Set default values
	this->SetDefaults();
}

// Called when the game starts or when spawned
void AFeatures::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
/*void AFeatures::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}*/

void AFeatures::SetDefaults()
{
	// Raw features
	this->user_location = FVector2D(0.0, 0.0);
	this->agv_location = FVector2D(0.0, 0.0);
	this->gaze_vector_3d = FVector(1.0, 0.0, 0.0);
	this->gaze_vector_2d = FVector2D(1.0, 0.0).GetSafeNormal();
	this->_user_yaw = 0.0;

	// Generated Features
	this->agv_to_user_distance = (this->user_location - this->agv_location).GetAbs();
	this->agv_speed = FVector2D(1.0, 1.0);
	this->user_velocity = FVector2D(0.0, 0.0);
	this->is_user_moving = false;
	//this->wait_time = 0.;
	this->intent_to_cross = false;
	this->gazing_station = -1;
	this->gazing_station_cos = -1.0;
	//bool possible_interaction;
	this->facing_sidewalk = false;
	this->facing_road = false;
	this->on_sidewalk = false;
	this->on_road = false;
	this->looking_at_agv = false;

	// Station info
	this->closest_station = 0;
	this->distance_to_closest_station = FVector2D(0.0, 0.0);
	this->start_station_id = 0;
	this->end_station_id = 1;
	this->distance_from_start_station = FVector2D(0.0, 0.0);
	this->distance_from_end_station = FVector2D(0.0, 0.0);
	this->facing_start_station = true;
	this->facing_end_station = false;

	// Frequency
	this->constants = Constants();
}


void AFeatures::SetRawFeatures(float user_x, float user_y, float agv_x, float agv_y,
	float gaze_vector_x, float gaze_vector_y, float gaze_vector_z, float user_yaw)
{
	this->user_location = FVector2D(user_x, user_y);
	this->agv_location = FVector2D(agv_x, agv_y);
	this->gaze_vector_3d = FVector(gaze_vector_x, gaze_vector_y, gaze_vector_z);
	this->gaze_vector_2d = FVector2D(gaze_vector_x, gaze_vector_y).GetSafeNormal();
	this->_user_yaw = user_yaw;
}


void AFeatures::SetStationInfo(int start_station, int end_station)
{
	this->start_station_id = start_station;
	this->end_station_id = end_station;
}

void AFeatures::GenerateRemainingFeatures(AFeatures* previous)
{
	this->agv_to_user_distance = (this->user_location - this->agv_location).GetAbs();
	this->agv_speed = (this->agv_location - previous->agv_location).GetAbs() * this->constants.FRAMERATE;
	this->user_velocity = (this->user_location - previous->user_location) * this->constants.FRAMERATE;
	this->is_user_moving = (user_velocity.Length() > this->constants.WALK_WAIT_THRESHOLD);

	// TODO: Change this when needed. We are currently not using the wait time feature for anything
	/*if (this->user_velocity.Length() < this->constants.WALK_WAIT_THRESHOLD)
	{
		this->wait_time = previous->wait_time + 1.0 / this->constants.FRAMERATE;
	}*/

	this->on_sidewalk = WithinSidewalkBounds(&this->user_location, this->constants.MARGIN_NEAR_SIDEWALKS);
	this->on_road = WithinRoadBounds(&this->user_location, this->constants.MARGIN_NEAR_SIDEWALKS / 2.0);
	
	// Closest station
	this->ClosestStationComputations();

	// Start and end station distance and facing computations
	this->StartAndEndStationsComputations();

	// Looking at AGV
	this->looking_at_agv = LookingAtAGV();
	
	// Facing the sidewalk
	this->facing_sidewalk = FacingSidewalk();

	// Facing the road
	this->facing_road = FacingRoad();
	
	// gazing_station and gazing_station_cos feature
	this->GazingStationComputations();

	// Intent to cross
	this->intent_to_cross = IntentToCrossComputations();

}

bool AFeatures::WithinRoadBounds(FVector2D* location, float error_range)
{
	if (location->Y < this->constants.SIDEWALK_1["Low"] - error_range &&
		location->Y > this->constants.SIDEWALK_2["High"] + error_range)
	{
		return true;
	}
	return false;
}

bool AFeatures::WithinSidewalkBounds(FVector2D* location, float error_range)
{	
	// If within the top sidewalk
	if (location->Y < this->constants.SIDEWALK_1["High"] + error_range && 
		location->Y > this->constants.SIDEWALK_1["Low"] - error_range)
	{
		return true;
	}

	// If within the bottom sidewalk
	if (location->Y < this->constants.SIDEWALK_2["High"] + error_range &&
		location->Y > this->constants.SIDEWALK_2["Low"] - error_range)
	{
		return true;
	}

	return false;
}

void AFeatures::ClosestStationComputations()
{
	float minimum_distance = INFINITY;
	int _closest_station = -1;
	for (auto& station : this->constants.STATIONS)
	{
		FVector2D* station_coords = &station.Value;
		float distance = (*station_coords - this->user_location).Length();
		if (distance < minimum_distance)
		{
			minimum_distance = distance;
			_closest_station = station.Key;
		}
	}
	this->closest_station = _closest_station;
	this->distance_to_closest_station = (this->constants.STATIONS[_closest_station] - this->user_location).GetAbs();
}

void AFeatures::StartAndEndStationsComputations()
{
	FVector2D* start_station_coords = &this->constants.STATIONS[this->start_station_id];
	FVector2D user_to_start = -(this->user_location - *start_station_coords);
	this->distance_from_start_station = user_to_start.GetAbs();

	// Angle between gaze vector 2d and the vector from start station to user should be small
	FVector2D normalized = user_to_start.GetSafeNormal();
	if (normalized.Dot(this->gaze_vector_2d) > this->constants.GAZING_ANGLE_THRESHOLD_COS)
	{
		this->facing_start_station = true;
	}
	else {
		this->facing_start_station = false;
	}

	FVector2D* end_station_coords = &this->constants.STATIONS[this->end_station_id];
	FVector2D user_to_end = -(this->user_location - *end_station_coords);
	this->distance_from_end_station = user_to_end.GetAbs();
	normalized = user_to_end.GetSafeNormal();
	if (normalized.Dot(this->gaze_vector_2d) > this->constants.GAZING_ANGLE_THRESHOLD_COS)
	{
		this->facing_end_station = true;
	}
	else {
		this->facing_end_station = false;
	}
}

bool AFeatures::LookingAtAGV()
{
	FVector2D user_to_agv = this->agv_location - this->user_location;
	FVector2D normalized = user_to_agv.GetSafeNormal();
	if (normalized.Dot(this->gaze_vector_2d) > this->constants.GAZING_ANGLE_THRESHOLD_COS)
	{
		return true;
	}
	return false;
}

bool AFeatures::FacingSidewalk()
{
	// If looking in either the positive or negative x direction
	if (abs(this->gaze_vector_2d.X) > this->constants.GAZING_ANGLE_THRESHOLD_COS)
	{
		return true;
	}

	return false;
}

bool AFeatures::FacingRoad()
{
	float road_midpoint = 0.5 * (this->constants.SIDEWALK_1["Low"] + this->constants.SIDEWALK_2["High"]);
	
	// IF ABOVE THE ROAD MIDPOINT AND LOOKING DOWN
	if (this->user_location.Y > road_midpoint && -this->gaze_vector_2d.Y > this->constants.GAZING_ANGLE_THRESHOLD_COS)
	{
		return true;
	}
	// IF BELOW THE ROAD MIDPOINT AND LOOKING UP
	if (this->user_location.Y < road_midpoint && this->gaze_vector_2d.Y > this->constants.GAZING_ANGLE_THRESHOLD_COS)
	{
		return true;
	}

	return false;
}

void AFeatures::GazingStationComputations()
{
	float maximum_cos = -1.0;
	int _gazing_station = -1;
	for (auto& station : this->constants.STATIONS)
	{
		FVector2D* station_coords = &station.Value;
		FVector2D user_to_station_normalized = (*station_coords - this->user_location).GetSafeNormal();
		float current_cos = user_to_station_normalized.Dot(this->gaze_vector_2d);
		if (current_cos > maximum_cos)
		{
			maximum_cos = current_cos;
			_gazing_station = station.Key;
		}
	}
	this->gazing_station = _gazing_station;
	this->gazing_station_cos = maximum_cos;
}

bool AFeatures::IntentToCrossComputations()
{
	if (this->gazing_station == -1)
	{
		this->GazingStationComputations();
	}

	if (this->gazing_station_cos > this->constants.GAZING_ANGLE_THRESHOLD_COS ||
		this->looking_at_agv)
	{
		return true;
	}
	
	return false;
}

bool AFeatures::copyFrom(AFeatures* Other)
{
	// Raw features
	this->user_location = Other->user_location;
	this->agv_location = Other->agv_location;
	this->gaze_vector_3d = Other->gaze_vector_3d;
	this->gaze_vector_2d = Other->gaze_vector_2d;
	this->_user_yaw = Other->_user_yaw;

	// Generated Features
	this->agv_to_user_distance = Other->agv_to_user_distance;
	this->agv_speed = Other->agv_speed;
	this->user_velocity = Other->user_velocity;
	this->is_user_moving = Other->is_user_moving;
	this->intent_to_cross = Other->intent_to_cross;
	this->gazing_station = Other->gazing_station;
	this->gazing_station_cos = Other->gazing_station_cos;
	this->facing_sidewalk = Other->facing_sidewalk;
	this->facing_road = Other->facing_road;
	this->on_sidewalk = Other->on_sidewalk;
	this->on_road = Other->on_road;
	this->closest_station = Other->closest_station;
	this->distance_to_closest_station = Other->distance_to_closest_station;
	this->looking_at_agv = Other->looking_at_agv;

	// Station info
	this->start_station_id = Other->start_station_id;
	this->end_station_id = Other->end_station_id;

	// Start and end station features
	this->distance_from_start_station = Other->distance_from_start_station;
	this->distance_from_end_station = Other->distance_from_end_station;
	this->facing_start_station = Other->facing_start_station;
	this->facing_end_station = Other->facing_end_station;


	return true;
}
