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
	this->wait_time = 0.f;
	this->intent_to_cross = false;
	this->gazing_station = -1;
	this->gazing_station_cos = -1.0;
	this->possible_interaction = false;
	this->facing_sidewalk = false;
	this->facing_road = false;
	this->on_sidewalk = false;
	this->on_road = false;
	this->looking_at_agv = false;

	// Station info
	this->closest_station = 0;
	this->gazing_station = 0;
	this->distance_to_closest_station = FVector2D(0.0, 0.0);

	this->begin_wait_flag = false;
	this->agv_passed = false;

	/*this->start_station_id = 0;
	this->end_station_id = 1;
	this->distance_from_start_station = FVector2D(0.0, 0.0);
	this->distance_from_end_station = FVector2D(0.0, 0.0);
	this->facing_start_station = true;
	this->facing_end_station = false;*/

	this->looking_at_closest_station = false;

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


/*void AFeatures::SetStationInfo(int start_station, int end_station)
{
	this->start_station_id = start_station;
	this->end_station_id = end_station;
}*/

void AFeatures::GenerateRemainingFeatures(AFeatures* previous)
{
	this->agv_to_user_distance = (this->user_location - this->agv_location).GetAbs();
	this->agv_speed = (this->agv_location - previous->agv_location).GetAbs() * this->constants.FRAMERATE;
	this->agv_velocity = (this->agv_location - previous->agv_location) * this->constants.FRAMERATE;
	this->user_velocity = (this->user_location - previous->user_location) * this->constants.FRAMERATE;
	this->is_user_moving = (user_velocity.Length() > this->constants.WALK_WAIT_THRESHOLD);

	/*// TODO: Change this when needed. We are currently not using the wait time feature for anything
	if (this->user_velocity.Length() < this->constants.WALK_WAIT_THRESHOLD)
	{
		this->wait_time = previous->wait_time + 1.0 / this->constants.FRAMERATE;
	}
	else {
		this->wait_time = 0.f;
	}*/

	this->on_sidewalk = WithinSidewalkBounds(&this->user_location, this->constants.MARGIN_NEAR_SIDEWALKS);
	this->on_road = WithinRoadBounds(&this->user_location, this->constants.MARGIN_NEAR_SIDEWALKS / 2.0);

	// Compute the wait time
	this->WaitTimeComputations(previous);

	// Closest station
	this->ClosestStationComputations();

	// Start and end station distance and facing computations
	//this->StartAndEndStationsComputations();

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

bool AFeatures::WithinRoadBounds(FVector2D* location, float error_range) const
{
	if (location->Y < this->constants.SIDEWALK_1["Low"] - error_range &&
		location->Y > this->constants.SIDEWALK_2["High"] + error_range)
	{
		return true;
	}
	return false;
}

bool AFeatures::WithinSidewalkBounds(FVector2D* location, float error_range) const
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

void AFeatures::PossibleInteractionComputations()
{
	float distance = this->agv_to_user_distance.Length(); // The distance between the AGV and the participant
	// If the distance between the participant and the AGV is lower than a threshold, set the possible interaction to be true and return
	if (distance < this->constants.COLLISION_THRESHOLD) {
		this->possible_interaction = true;
		return;
	}
	// Compute the relative velocity between the participant and the AGV
	FVector2D relative_velocity = this->user_velocity - this->agv_velocity;
	float relative_speed = relative_velocity.Length();
	
	// If there is very little relative velocity between the participant and the AGV, there would be no interaction between them
	if (relative_speed < this->constants.WALK_WAIT_THRESHOLD)
	{
		this->possible_interaction = false;
		return;
	}

	// Compute the time at which the distance between the AGV and the participant will be minimum
	// assuming that they continue at the same velocities
	FVector2D agv_to_user_vector = this->user_location - this->agv_location;
	float time_at_closest_point = -agv_to_user_vector.Dot(relative_velocity) / (relative_speed * relative_speed);
	
	// If this time is positive, the distance between the AGV and the participant is decreasing
	if (time_at_closest_point > 0)
	{
		FVector2D future_user_location = this->user_location + this->user_velocity * time_at_closest_point;
		FVector2D future_agv_location = this->agv_location + this->agv_velocity * time_at_closest_point;
		distance = (future_user_location - future_agv_location).Length();
		if (distance < this->constants.COLLISION_THRESHOLD)
		{
			this->possible_interaction = true;
			return;
		}
	}
	
	// If all else fails
	this->possible_interaction = false;
	return;
}

void AFeatures::WaitTimeComputations(AFeatures* previous)
{
	// If the agv has already passed, set the wait time to 0 and return
	this->agv_passed = previous->agv_passed;
	if (this->agv_passed)
	{
		this->wait_time = 0.f;
		return;
	}

	// Else, if the agv has not passed, do additional computations

	// If the user was walking in the previous timestep
	if (!previous->begin_wait_flag)
	{
		// If the user is stopped on the sidewalk
		if (!this->is_user_moving && this->on_sidewalk)
		{
			this->begin_wait_flag = true;
			this->wait_time = previous->wait_time + 1.f / this->constants.FRAMERATE;
		}
		else 
		{
			this->wait_time = 0;
			this->begin_wait_flag = false;
		}
	}
	else //User was in the wait state in the previous timestep
	{
		if (!this - is_user_moving)
		{
			this->wait_time = previous->wait_time + 1.f / this->constants.FRAMERATE;
		}
		else
		{
			this->begin_wait_flag = false;
			this->wait_time = 0.f;
			this->agv_passed = true;
		}
	}

}

void AFeatures::ClosestStationComputations()
{
	float minimum_distance = (float) 1e20;
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

/*void AFeatures::StartAndEndStationsComputations()
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
}*/

bool AFeatures::LookingAtAGV() const
{
	FVector2D user_to_agv = this->agv_location - this->user_location;
	FVector2D normalized = user_to_agv.GetSafeNormal();
	if (normalized.Dot(this->gaze_vector_2d) > this->constants.GAZING_ANGLE_THRESHOLD_COS)
	{
		return true;
	}
	return false;
}

bool AFeatures::FacingSidewalk() const
{
	// If looking in either the positive or negative x direction
	if (abs(this->gaze_vector_2d.X) > this->constants.GAZING_ANGLE_THRESHOLD_COS)
	{
		return true;
	}

	return false;
}

bool AFeatures::FacingRoad() const
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
	this->looking_at_closest_station = (this->gazing_station == this->closest_station);
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
	this->wait_time = Other -> wait_time;
	this->possible_interaction = Other->possible_interaction;
	this->looking_at_closest_station = Other->looking_at_closest_station;

	// Station info
	//this->start_station_id = Other->start_station_id;
	//this->end_station_id = Other->end_station_id;

	// Start and end station features
	//this->distance_from_start_station = Other->distance_from_start_station;
	//this->distance_from_end_station = Other->distance_from_end_station;
	//this->facing_start_station = Other->facing_start_station;
	//this->facing_end_station = Other->facing_end_station;


	return true;
}


void AFeatures::CreateInputArray(UPARAM(Ref) TArray<float>& FullModelInput, int TimestampID)
{

	checkf(TimestampID < 30, TEXT("More than 30 Timestamp ID inputted to the feature array converter"));
	checkf(FullModelInput.Num() == 930, TEXT("Incorrect length of Feature Array. Expected 930"));
	int start_idx = TimestampID * 31;
	FullModelInput[start_idx + 0] = user_location.X;
	FullModelInput[start_idx + 1] = user_location.Y;
	FullModelInput[start_idx + 2] = agv_to_user_distance.X;
	FullModelInput[start_idx + 3] = agv_to_user_distance.Y;
	FullModelInput[start_idx + 4] = agv_speed.X;
	FullModelInput[start_idx + 5] = agv_speed.Y;
	FullModelInput[start_idx + 6] = agv_speed.Length();
	FullModelInput[start_idx + 7] = abs(user_velocity.X);
	FullModelInput[start_idx + 8] = abs(user_velocity.Y);
	FullModelInput[start_idx + 9] = user_velocity.Length();
	FullModelInput[start_idx + 10] = user_velocity.X;
	FullModelInput[start_idx + 11] = user_velocity.Y;
	FullModelInput[start_idx + 12] = wait_time;
	FullModelInput[start_idx + 13] = intent_to_cross;
	FullModelInput[start_idx + 14] = gazing_station;
	FullModelInput[start_idx + 15] = possible_interaction;
	FullModelInput[start_idx + 16] = facing_sidewalk;
	FullModelInput[start_idx + 17] = facing_road;
	FullModelInput[start_idx + 18] = on_sidewalk;
	FullModelInput[start_idx + 19] = on_road;
	FullModelInput[start_idx + 20] = closest_station;
	FullModelInput[start_idx + 21] = distance_to_closest_station.Length();
	FullModelInput[start_idx + 22] = distance_to_closest_station.X;
	FullModelInput[start_idx + 23] = distance_to_closest_station.Y;
	FullModelInput[start_idx + 24] = looking_at_agv;
	FullModelInput[start_idx + 25] = gaze_vector_3d.X;
	FullModelInput[start_idx + 26] = gaze_vector_3d.Y;
	FullModelInput[start_idx + 27] = gaze_vector_3d.Z;
	FullModelInput[start_idx + 28] = agv_location.X;
	FullModelInput[start_idx + 29] = agv_location.Y;
	FullModelInput[start_idx + 30] = looking_at_closest_station;
}

void AFeatures::PrintPositions(const TArray<float>& ModelOutput)
{
	//checkf(ModelOutput.Num() == 80, TEXT("Expected an array size of 80"));
	for (int i = 0; (int) i < ModelOutput.Num() / 2; i++)
	{
		//FString out = FString::Printf("(%.2f, %.2f)", ModelOutput[2 * i], ModelOutput[2 * i + 1]);
		UE_LOG(LogTemp, Display, TEXT("(%.2f, %.2f)"), ModelOutput[2 * i], ModelOutput[2 * i + 1]);
	}
}