// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Constants.h"
#include "GameFramework/Actor.h"
#include "Features.generated.h"

UCLASS(Blueprintable)
class FINITEAUTOMATONMODEL_API AFeatures : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFeatures();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

// public:	
	// Called every frame
	// virtual void Tick(float DeltaTime) override;

public:
	// Raw features
	FVector2D user_location;
	FVector2D agv_location;
	FVector gaze_vector_3d;
	FVector2D gaze_vector_2d;
	float _user_yaw;                // NOT USED. USING GAZE INSTEAD

	// Generated Features
	FVector2D agv_to_user_distance;

	UPROPERTY(BlueprintReadOnly)
	FVector2D agv_speed;

	UPROPERTY(BlueprintReadOnly)
	FVector2D user_velocity;

	UPROPERTY(BlueprintReadOnly)
	bool is_user_moving;
	//float wait_time;
	bool intent_to_cross;
	int gazing_station;
	float gazing_station_cos;
	//bool possible_interaction;
	bool facing_sidewalk;
	bool facing_road;
	bool on_sidewalk;
	bool on_road;
	int closest_station;
	FVector2D distance_to_closest_station;
	bool looking_at_agv;

	// Station info
	int start_station_id;
	int end_station_id;

	// Start and end station features
	FVector2D distance_from_start_station;
	FVector2D distance_from_end_station;
	bool facing_start_station;
	bool facing_end_station;

	// constants
	Constants constants;


public:

	// This will be called every time new data is generated
	UFUNCTION(BlueprintCallable)
	void SetRawFeatures(float user_x, float user_y, float agv_x, float agv_y,
		float gaze_vector_x, float gaze_vector_y, float gaze_vector_z, float user_yaw);
	
	// This will be called once per trajectory (or AGV)
	UFUNCTION(BlueprintCallable)
	void SetStationInfo(int start_station, int end_station);

	UFUNCTION(BlueprintCallable)
	void GenerateRemainingFeatures(AFeatures* previous);

	UFUNCTION(BlueprintCallable)
	bool copyFrom(AFeatures* Other);


private:
	void SetDefaults();
	UObject* GameInstance;
	bool WithinSidewalkBounds(FVector2D* location, float error_range);
	bool WithinRoadBounds(FVector2D* location, float error_range);
	void ClosestStationComputations();
	void StartAndEndStationsComputations();
	bool LookingAtAGV();
	bool FacingSidewalk();
	bool FacingRoad();
	void GazingStationComputations();
	bool IntentToCrossComputations();

};