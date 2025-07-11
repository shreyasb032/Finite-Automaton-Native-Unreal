// Fill out your copyright notice in the Description page of Project Settings.


#include "FiniteAutomaton.h"

// Sets default values
AFiniteAutomaton::AFiniteAutomaton()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Assumes that the starting state is error state
	this->current_state_id = -1;
	this->prev_state_id = -1;
	this->constants = Constants();
	this->error_flags = { true, true, false };
	this->idx_to_name.Add(-1, "Error");
	this->idx_to_name.Add(0, "At Station");
	this->idx_to_name.Add(1, "Approach Sidewalk");
	this->idx_to_name.Add(2, "Wait");
	this->idx_to_name.Add(3, "Move Along Sidewalk");
	this->idx_to_name.Add(4, "Cross");
	this->idx_to_name.Add(5, "Approach Target Station");
	this->current_state_name = this->idx_to_name[this->current_state_id];
		
	// Manually added the MLE transition probabilties
	this->MLE.Add(0, { 0.950089, 0.018987, 0.000000, 0.000000, 0.252874, 0.0000 });
	this->MLE.Add(1, { 0.023619, 0.126582, 0.562738, 0.000000, 0.000000, 0.0050 });
	this->MLE.Add(2, { 0.022282, 0.805907, 0.087452, 0.015152, 0.000000, 0.0075 });
	this->MLE.Add(3, { 0.000446, 0.008439, 0.087452, 0.003367, 0.004598, 0.9200 });
	this->MLE.Add(4, { 0.000000, 0.040084, 0.262357, 0.850168, 0.000000, 0.0025 });
	this->MLE.Add(5, { 0.003565, 0.000000, 0.000000, 0.131313, 0.000000, 0.0650 });
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
	if (this->current_state_id == -1)
	{
		this->current_state_id = this->HandleErrorState(current);
	}
	// If the current state is At Station, handle it
	else if (this->current_state_id == 0)
	{
		this->current_state_id = this->HandleAtStationState(current);
	}
	// If the current state is Approach Sidewalk, handle it
	else if (this->current_state_id == 1) {
		this->current_state_id = this->HandleApproachSidewalkState(current);
	}
	// If the current state is Wait, handle it
	else if (this->current_state_id == 2) {
		this->current_state_id = this->HandleWaitState(current);
	}
	// If the current state is Move Along Sidewalk, handle it
	else if (this->current_state_id == 3) {
		this->current_state_id = this->HandleMovingAlongSidewalkState(current);
	}
	// If the current state is Cross, handle it
	else if (this->current_state_id == 4) {
		this->current_state_id = this->HandleCrossState(current);
	}
	// If the current state is Approach Target Station, handle it
	else if (this->current_state_id == 5) {
		this->current_state_id = this->HandleApproachStationState(current);
	}
	else {
		// Do nothing
	}

	// Error handling. For the new current state, check if the constraints are satisfied. If not, add to the error pile. 
	// If the error pile is full, change the current state to Error
	this->current_state_id = ConstraintChecking(current);
	this->current_state_name = this->idx_to_name[this->current_state_id];

	if (this->current_state_id != -1) {
		this->prev_state_id = this->current_state_id;
	}

	return this->current_state_name;
}

bool AFiniteAutomaton::reset()
{
	this->current_state_id = -1;
	this->current_state_name = this->idx_to_name[this->current_state_id];
	this->error_flags = { true, true, false };

	return true;
}

bool AFiniteAutomaton::AtStationStateChecker(AFeatures* current) const
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

	if (moving && on_road && (facing_road || looking_at_agv)) return true;
	//if (moving && on_road) return true;

	return false;
}

bool AFiniteAutomaton::ApproachingSidewalkStateChecker(AFeatures* current) const
{
	//bool near_start_station = current->distance_from_start_station.X < this->constants.STATION_LENGTH;
	//near_start_station = near_start_station && current->distance_from_start_station.Y < this->constants.NEAR_STATION_THRESHOLD;
	bool near_start_station = current->distance_to_closest_station.Y <= this->constants.NEAR_STATION_THRESHOLD;
	//bool facing_start_station = current->facing_start_station;
	bool moving = current->is_user_moving;

	if (near_start_station /* && !facing_start_station*/ && moving && !current->on_road) return true;

	return false;
}

bool AFiniteAutomaton::MovingAlongSidewalkStateChecker(AFeatures* current) const
{
	bool moving = current->is_user_moving;
	bool on_sidewalk = current->on_sidewalk;
	bool facing_sidewalk = current->facing_sidewalk;

	if (moving && on_sidewalk && facing_sidewalk) return true;

	return false;
}

bool AFiniteAutomaton::ApproachingStationStateChecker(AFeatures* current) const
{
	//bool near_end_station_x = current->distance_from_end_station.X < this->constants.STATION_LENGTH;
	//bool near_end_station = near_end_station_x && current->distance_from_end_station.Y < this->constants.NEAR_STATION_THRESHOLD;
	//bool at_end_station = near_end_station_x && current->distance_from_end_station.Y < this->constants.AT_STATION_THRESHOLD;

	bool moving = current->user_velocity.Length() > this->constants.WALK_WAIT_THRESHOLD * 0.2;
	bool facing_end_station = current->closest_station == current->gazing_station;
	bool near_end_station = current->distance_to_closest_station.Length() < this->constants.AT_STATION_THRESHOLD * 2;

	if (!current->on_road && moving && near_end_station && facing_end_station) return true;

	return false;
}


int AFiniteAutomaton::HandleErrorState(AFeatures* current)
{
	// If we had an error state previously, then go through a fixed order
	// This should only happen at the first time step
	if (prev_state_id == -1) {
		if (AtStationStateChecker(current)) return 0;

		if (WaitingStateChecker(current)) return 2;

		if (CrossingStateChecker(current)) return 4;

		if (ApproachingSidewalkStateChecker(current)) return 1;

		if (MovingAlongSidewalkStateChecker(current)) return 3;

		if (ApproachingStationStateChecker(current)) return 5;

		return -1;
	}
	// Otherwise, we have a valid previous state. We need to do constraint checking in the order
	// of decreasing probabilities of transitions
	// STEP 1: Select the correct row of MLE
	TArray<float> transition_probs = this->MLE[this->prev_state_id];

	// STEP 2: Create a boolean array with all constraint checks
	TArray<TTuple<int, bool, float>> constraint_checks;
	constraint_checks.Add(MakeTuple(0, AtStationStateChecker(current), transition_probs[0]));
	constraint_checks.Add(MakeTuple(1, ApproachingSidewalkStateChecker(current), transition_probs[1]));
	constraint_checks.Add(MakeTuple(2, WaitingStateChecker(current), transition_probs[2]));
	constraint_checks.Add(MakeTuple(3, MovingAlongSidewalkStateChecker(current), transition_probs[3]));
	constraint_checks.Add(MakeTuple(4, CrossingStateChecker(current), transition_probs[4]));
	constraint_checks.Add(MakeTuple(5, ApproachingStationStateChecker(current), transition_probs[5]));

	// STEP 3: Sort the array according to the probabilities
	constraint_checks.Sort([](const TTuple<int, bool, float> A, const TTuple<int, bool, float> B) {
		return  A.Get<2>() > B.Get<2>();
		});

	// STEP 4: Go through the sorted array and return the first state whose constraints are satisfied
	for (TTuple<int, bool, float> constraint_check : constraint_checks) {
		if (constraint_check.Get<1>()) {
			return constraint_check.Get<0>();
		}
	}

	return -1;
}

int AFiniteAutomaton::HandleAtStationState(AFeatures* current) const
{
	// Transition to Approaching Sidewalk State
	if (current->is_user_moving && (current->on_sidewalk || current->facing_sidewalk)) return 1;

	// Transition to Waiting State
	if (!current->is_user_moving && current->intent_to_cross /* && current->possible_interaction*/) return 2;

	return 0;
}

int AFiniteAutomaton::HandleApproachSidewalkState(AFeatures* current) const
{
	// Transition to Cross
	if (current->is_user_moving && current->facing_road && current->on_road) return 4;

	// Transition to Wait
	if (!current->is_user_moving && current->intent_to_cross /*&& current->possible_interaction*/) return 2;

	// Transition to Move Along Sidewalk
	FVector2D user_speed = current->user_velocity.GetAbs();
	bool moving_along_x = user_speed.X > 1.5 * user_speed.Y;
	if (moving_along_x || (current->is_user_moving && current->facing_sidewalk)) return 3;

	return 1;
}

int AFiniteAutomaton::HandleWaitState(AFeatures* current) const
{

	// Transition to Cross (TODO: Facing road may cause problems here)
	bool moving_y = abs(current->user_velocity.Y) > this->constants.WALK_WAIT_THRESHOLD;
	if (moving_y && current->on_road /*&& current->facing_road*/) return 4;

	// Transition to Approach Sidewalk (TODO: Instead of on_sidewalk, some measure of near station may be better)
	if (moving_y && current->on_sidewalk) return 1;

	// Transition to Move Along Sidewalk
	bool moving_x = abs(current->user_velocity.X) > 0.8 * this->constants.WALK_WAIT_THRESHOLD;
	if (moving_x && (current->facing_sidewalk || current->on_sidewalk)) return 3;

	return 2;
}

int AFiniteAutomaton::HandleMovingAlongSidewalkState(AFeatures* current) const
{
	// Transition to Cross
	bool moving_y = abs(current->user_velocity.Y) > this->constants.WALK_WAIT_THRESHOLD;
	bool condition1 = moving_y && current->facing_road;
	bool condition2 = abs(current->user_velocity.Y) > 1.5 * abs(current->user_velocity.X);
	bool condition3 = current->intent_to_cross || current->on_road;
	if ((condition1 || condition2) && condition3) return 4;

	// Transition to Wait
	if (!current->is_user_moving && current->intent_to_cross /*&& current->possible_interaction*/) return 2;

	// Transition to Approaching Station State
	/*bool near_end_station_x = current->distance_from_end_station.X < this->constants.STATION_LENGTH;
	bool near_end_station = near_end_station_x && current->distance_from_end_station.Y < this->constants.NEAR_STATION_THRESHOLD;
	if (current->is_user_moving && !current->facing_road && near_end_station) return 5;*/
	condition1 = current->user_velocity.Length() < this->constants.WALK_WAIT_THRESHOLD;
	condition1 = condition1 || (current->closest_station == current->gazing_station);
	condition2 = !current->facing_road;
	condition3 = current->distance_to_closest_station.Length() < this->constants.AT_STATION_THRESHOLD * 2;
	if (condition1 && condition2 && condition3) return 5;

	return 3;
}

int AFiniteAutomaton::HandleCrossState(AFeatures* current) const
{
	// Transition to Move Along Sidewalk
	bool moving_x = abs(current->user_velocity.X) > this->constants.WALK_WAIT_THRESHOLD;
	if (current->on_sidewalk && moving_x) return 3;

	// Transition to Approach Target Station
	//bool near_end_station_x = current->distance_from_end_station.X < this->constants.STATION_LENGTH;
	//bool near_end_station = near_end_station_x && current->distance_from_end_station.Y < this->constants.NEAR_STATION_THRESHOLD;
	bool near_end_station = (current->closest_station == current->gazing_station);
	if (current->is_user_moving && !current->on_road && near_end_station) return 5;

	// Transition to Wait
	if (!current->is_user_moving && current->looking_at_agv && current->on_road /*&& current->possible_interaction*/) return 2;

	// Transition to At Station
	near_end_station = current->distance_to_closest_station.Length() <= this->constants.AT_STATION_THRESHOLD;
	if (!current->is_user_moving && !current->facing_road && near_end_station) return 0;

	return 2;
}

int AFiniteAutomaton::HandleApproachStationState(AFeatures* current) const
{
	// Transition to At Station
	if (!current->is_user_moving) return 0;

	return 5;
}

int AFiniteAutomaton::ConstraintChecking(AFeatures* current)
{
	bool error;
	if (this->current_state_id == 0)
	{
		error = !AtStationStateChecker(current);
	}
	// If the current state is Approach Sidewalk, handle it
	else if (this->current_state_id == 1) {
		error = !ApproachingSidewalkStateChecker(current);
	}
	// If the current state is Wait, handle it
	else if (this->current_state_id == 2) {
		error = !WaitingStateChecker(current);
	}
	// If the current state is Move Along Sidewalk, handle it
	else if (this->current_state_id == 3) {
		error = !MovingAlongSidewalkStateChecker(current);
	}
	// If the current state is Cross, handle it
	else if (this->current_state_id == 4) {
		error = !CrossingStateChecker(current);
	}
	// If the current state is Approach Target Station, handle it
	else if (this->current_state_id == 5) {
		error = !ApproachingStationStateChecker(current);
	}
	else { // Error state
		error = true;
	}

	this->error_flags.RemoveAt(0);
	this->error_flags.Add(error);

	for (bool err : this->error_flags) {
		// If any non-error is encountered, return the current state as is
		if (!err) {
			return this->current_state_id;
		}
	}

	// If all three values are true, return the error state
	return -1;
}
