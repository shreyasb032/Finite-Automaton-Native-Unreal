// Fill out your copyright notice in the Description page of Project Settings.


#include "FiniteAutomaton.h"

// Sets default values
AFiniteAutomaton::AFiniteAutomaton()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Assumes that the starting state is error state
	this->CurrentState = "Error";
	this->constants = Constants();
	this->error_flags = { true, true, false };
}

// Called when the game starts or when spawned
void AFiniteAutomaton::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
/*void AFiniteAutomaton::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}*/

FString AFiniteAutomaton::GetState(AFeatures* current)
{
	// If the current state is error, run state checker to return the state that satisfies the constraints
	if (this->CurrentState.Compare("Error") == 0)
	{
		this->CurrentState = this->HandleErrorState(current);
	}
	// If the current state is Start, handle it
	else if (this->CurrentState.Compare("Start") == 0)
	{
		this->CurrentState = this->HandleStartState(current);
	}
	// If the current state is Approach Sidewalk, handle it
	else if (this->CurrentState.Compare("Approach Sidewalk") == 0) {
		this->CurrentState = this->HandleApproachSidewalkState(current);
	}
	// If the current state is Wait, handle it
	else if (this->CurrentState.Compare("Wait") == 0) {
		this->CurrentState = this->HandleWaitState(current);
	}
	// If the current state is Move Along Sidewalk, handle it
	else if (this->CurrentState.Compare("Move Along Sidewalk") == 0) {
		this->CurrentState = this->HandleMovingAlongSidewalkState(current);
	}
	// If the current state is Cross, handle it
	else if (this->CurrentState.Compare("Cross") == 0) {
		this->CurrentState = this->HandleCrossState(current);
	}
	// If the current state is Approach Target Station, handle it
	else if (this->CurrentState.Compare("Approach Target Station") == 0) {
		this->CurrentState = this->HandleApproachStationState(current);
	}
	// if the current state is Arrived, handle it (this does not do anything, just here to maintain consistency)
	else if (this->CurrentState.Compare("Arrived") == 0) {
		this->CurrentState = this->HandleArrivedState(current);
	}

	// Error handling. For the new current state, check if the constraints are satisfied. If not, add to the error pile. 
	// If the error pile is full, change the current state to Error
	this->CurrentState = ConstraintChecking(this->CurrentState, current);

	return this->CurrentState;
}

bool AFiniteAutomaton::reset()
{
	this->CurrentState = "Error";
	this->error_flags = { true, true, false };

	return true;
}

bool AFiniteAutomaton::StartStateChecker(AFeatures* current)
{
	bool near_start_station = current->distance_from_start_station.X < this->constants.STATION_LENGTH;
	near_start_station = near_start_station && current->distance_from_start_station.Y < this->constants.AT_STATION_THRESHOLD;

	bool facing_start_station = current->facing_start_station;

	bool stationary = current->user_velocity.Length() < this->constants.WALK_WAIT_THRESHOLD;

	if (stationary && near_start_station && facing_start_station) return true;

	return false;
}

bool AFiniteAutomaton::WaitingStateChecker(AFeatures* current)
{
	bool stationary = !current->is_user_moving;
	bool facing_road = current->facing_road;
	bool looking_at_agv = current->looking_at_agv;
	// return stationary
	return stationary && (facing_road || looking_at_agv);
}

bool AFiniteAutomaton::CrossingStateChecker(AFeatures* current)
{
	bool moving = current->is_user_moving;
	bool on_road = current->on_road;
	bool facing_road = current->facing_road;
	bool looking_at_agv = current->looking_at_agv;

	//if (moving && on_road && (facing_road || looking_at_agv)) return true;
	if (moving && on_road) return true;

	return false;
}

bool AFiniteAutomaton::ApproachingSidewalkStateChecker(AFeatures* current)
{
	bool near_start_station = current->distance_from_start_station.X < this->constants.STATION_LENGTH;
	near_start_station = near_start_station && current->distance_from_start_station.Y < this->constants.NEAR_STATION_THRESHOLD;
	bool facing_start_station = current->facing_start_station;
	bool moving = current->is_user_moving;

	if (near_start_station && !facing_start_station && moving) return true;

	return false;
}

bool AFiniteAutomaton::MovingAlongSidewalkStateChecker(AFeatures* current)
{
	bool moving = current->is_user_moving;
	bool on_sidewalk = current->on_sidewalk;
	bool facing_sidewalk = current->facing_sidewalk;

	if (moving && on_sidewalk && facing_sidewalk) return true;

	return false;
}

bool AFiniteAutomaton::ApproachingStationStateChecker(AFeatures* current)
{
	bool near_end_station_x = current->distance_from_end_station.X < this->constants.STATION_LENGTH;
	bool near_end_station = near_end_station_x && current->distance_from_end_station.Y < this->constants.NEAR_STATION_THRESHOLD;
	//bool at_end_station = near_end_station_x && current->distance_from_end_station.Y < this->constants.AT_STATION_THRESHOLD;

	bool moving = current->is_user_moving;
	bool facing_end_station = current->facing_end_station;

	if (moving && near_end_station && facing_end_station) return true;

	return false;
}

bool AFiniteAutomaton::ArrivedStateChecker(AFeatures* current)
{
	bool near_end_station_x = current->distance_from_end_station.X < this->constants.STATION_LENGTH;
	bool near_end_station = near_end_station_x && current->distance_from_end_station.Y < this->constants.NEAR_STATION_THRESHOLD;
	bool moving = current->is_user_moving;

	if (!moving && near_end_station) return true;

	return false;
}

FString AFiniteAutomaton::HandleErrorState(AFeatures* current)
{
	if (StartStateChecker(current)) return "Start";

	if (WaitingStateChecker(current)) return "Wait";

	if (CrossingStateChecker(current)) return "Cross";

	if (ApproachingSidewalkStateChecker(current)) return "Approach Sidewalk";

	if (MovingAlongSidewalkStateChecker(current)) return "Move Along Sidewalk";

	if (ApproachingStationStateChecker(current)) return "Approach Target Station";

	if (ArrivedStateChecker(current)) return "Arrived";

	return "Error";
}

FString AFiniteAutomaton::HandleStartState(AFeatures* current)
{
	// Transition to Approaching Sidewalk State
	if (current->is_user_moving && (current->on_sidewalk || current->facing_sidewalk)) return "Approach Sidewalk";

	// Transition to Waiting State
	if (!current->is_user_moving && current->intent_to_cross) return "Wait";

	return "Start";
}

FString AFiniteAutomaton::HandleApproachSidewalkState(AFeatures* current)
{
	// Transition to Cross
	if (current->is_user_moving && current->facing_road) return "Cross";

	// Transition to Wait
	if (!current->is_user_moving && current->intent_to_cross) return "Wait";

	// Transition to Move Along Sidewalk
	if (current->is_user_moving && current->facing_sidewalk) return "Move Along Sidewalk";

	return "Approach Sidewalk";
}

FString AFiniteAutomaton::HandleWaitState(AFeatures* current)
{

	// Transition to Cross (TODO: Facing road may cause problems here)
	bool moving_y = abs(current->user_velocity.Y) > this->constants.WALK_WAIT_THRESHOLD;
	if (moving_y && current->on_road /*&& current->facing_road*/) return "Cross";

	// Transition to Approach Sidewalk (TODO: Instead of on_sidewalk, some measure of near station may be better)
	if (moving_y && current->on_sidewalk) return "Approach Sidewalk";

	// Transition to Move Along Sidewalk
	bool moving_x = abs(current->user_velocity.X) > this->constants.WALK_WAIT_THRESHOLD;
	if (moving_x && (current->facing_sidewalk || current->on_sidewalk)) return "Move Along Sidewalk";

	return "Wait";
}

FString AFiniteAutomaton::HandleMovingAlongSidewalkState(AFeatures* current)
{
	// Transition to Cross
	bool moving_y = abs(current->user_velocity.Y) > this->constants.WALK_WAIT_THRESHOLD;
	if (moving_y && current->facing_road) return "Cross";

	// Transition to Wait
	if (!current->is_user_moving && current->intent_to_cross) return "Wait";

	// Transition to Approach Target State
	bool near_end_station_x = current->distance_from_end_station.X < this->constants.STATION_LENGTH;
	bool near_end_station = near_end_station_x && current->distance_from_end_station.Y < this->constants.NEAR_STATION_THRESHOLD;
	if (current->is_user_moving && !current->facing_road && near_end_station) return "Approach Target Station";

	return "Move Along Sidewalk";
}

FString AFiniteAutomaton::HandleCrossState(AFeatures* current)
{
	// Transition to Move Along Sidewalk
	bool moving_x = abs(current->user_velocity.X) > this->constants.WALK_WAIT_THRESHOLD;
	if (current->on_sidewalk && moving_x) return "Move Along Sidewalk";

	// Transition to Approach Target Station
	bool near_end_station_x = current->distance_from_end_station.X < this->constants.STATION_LENGTH;
	bool near_end_station = near_end_station_x && current->distance_from_end_station.Y < this->constants.NEAR_STATION_THRESHOLD;
	if (current->is_user_moving && !current->facing_road && near_end_station) return "Approach Target Station";

	// Transition to Wait
	if (!current->is_user_moving && current->looking_at_agv && current->on_road) return "Wait";

	// Transition to Arrived
	if (!current->is_user_moving && !current->facing_road && near_end_station) return "Arrived";

	return "Cross";
}

FString AFiniteAutomaton::HandleApproachStationState(AFeatures* current)
{
	// Transition to Arrived
	if (!current->is_user_moving) return "Arrived";

	return "Approach Target Station";
}

FString AFiniteAutomaton::HandleArrivedState(AFeatures* current)
{
	return "Arrived";
}

FString AFiniteAutomaton::ConstraintChecking(FString current_state, AFeatures* current)
{
	bool error;
	if (current_state.Compare("Start") == 0)
	{
		error = !StartStateChecker(current);
	}
	// If the current state is Approach Sidewalk, handle it
	else if (current_state.Compare("Approach Sidewalk") == 0) {
		error = !ApproachingSidewalkStateChecker(current);
	}
	// If the current state is Wait, handle it
	else if (current_state.Compare("Wait") == 0) {
		error = !WaitingStateChecker(current);
	}
	// If the current state is Move Along Sidewalk, handle it
	else if (current_state.Compare("Move Along Sidewalk") == 0) {
		error = !MovingAlongSidewalkStateChecker(current);
	}
	// If the current state is Cross, handle it
	else if (current_state.Compare("Cross") == 0) {
		error = !CrossingStateChecker(current);
	}
	// If the current state is Approach Target Station, handle it
	else if (current_state.Compare("Approach Target Station") == 0) {
		error = !ApproachingStationStateChecker(current);
	}
	// if the current state is Arrived, handle it (this does not do anything, just here to maintain consistency)
	else if (current_state.Compare("Arrived") == 0) {
		error = !ArrivedStateChecker(current);
	}
	else { // Error state
		error = true;
	}

	this->error_flags.RemoveAt(0);
	this->error_flags.Add(error);

	for (bool err : this->error_flags) {
		// If any non-error is encountered, return the current state as is
		if (!err) {
			return current_state;
		}
	}

	// If all three values are true, return the error state
	return "Error";
}
