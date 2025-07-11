// Fill out your copyright notice in the Description page of Project Settings.


#include "DataReader.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"

// Sets default values
ADataReader::ADataReader()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	this->line_number = 1;
	this->num_lines = -1;
	this->header_written = false;
	this->state_header_written = false;
	this->agv_num = 0;
	this->reset_flag = false;
}

// Called when the game starts or when spawned
void ADataReader::BeginPlay()
{
	Super::BeginPlay();
	
}

bool ADataReader::ReadData()
{
	FString temp_filename = "PID";
	int _pid = FCString::Atoi(*this->PID);
	if (_pid < 10)
	{
		temp_filename += "00" + this->PID;
	}
	else {
		temp_filename += "0" + this->PID;
	}

	temp_filename += "_" + this->SCN + ".csv";

	this->filename = FPaths::Combine(FPaths::ProjectDir(), "Data", "RawData", temp_filename); // Filename looks good
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Filename %s"), *this->filename));

	bool read = FFileHelper::LoadFileToStringArray(this->data, *this->filename);

	this->num_lines = this->data.Num() - 1;
	this->header_written = false;
	this->state_header_written = false;
	
	/*this->header = "user_x, user_y, agv_x, agv_y, gaze_vector_x, gaze_vector_y, gaze_vector_z, user_yaw, ";
	this->header += "agv_user_distance_x, agv_user_distance_y, agv_speed_x, agv_speed_y, user_velocity_x, user_velocity_y, is_user_moving, ";
	this->header += "intent_to_cross, gazing_station, gazing_station_cos, facing_sidewalk, facing_road, on_sidewalk, on_road, ";
	this->header += "closest_station, distance_to_closest_station_x, distance_to_closest_station_y, looking_at_agv, ";*/
	/*this->header += "start_station_id, end_station_id, distance_from_start_station_x, distance_from_start_station_y, ";
	this->header += "distance_from_end_station_x, distance_from_end_station_y, facing_start_station, facing_end_station";*/

	this->header = "AGV_name, User_X, User_Y, AGV_distance_X, AGV_distance_Y, AGV_speed_X,";
	this->header += "AGV_speed_Y, AGV_speed, User_speed_X, User_speed_Y,";
	this->header += "User_speed, User_velocity_X, User_velocity_Y, Wait_time,";
	this->header += "intent_to_cross, Gazing_station, possible_interaction,";
	this->header += "facing_along_sidewalk, facing_to_road, On_sidewalks, On_road,";
	this->header += "closest_station, distance_to_closest_station,";
	this->header += "distance_to_closest_station_X, distance_to_closest_station_Y,";
	this->header += "looking_at_AGV, GazeDirection_X, GazeDirection_Y,";
	this->header += "GazeDirection_Z, AGV_X, AGV_Y, looking_at_closest_station";

	return read;
}

void ADataReader::GetOneLine(AFeatures* features, int line_num = -1)
{
	bool to_increment = false;
	if (line_num == -1) {
		line_num = this->line_number;
		to_increment = true;
	}

	if (to_increment)
	{
		this->line_number++;
	}

	if (line_num > this->num_lines) {
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Line number %d greater than number of lines %d"), line_num, this->num_lines));
		FGenericPlatformMisc::RequestExit(true);
		return;
	}

	FString row = this->data[line_num];
	float user_x, user_y, agv_x, agv_y, gaze_x, gaze_y, gaze_z;
	TArray<FString> splitted;
	FString delimiter = ",";
	row.ParseIntoArray(splitted, *delimiter);

	FString agv_name = splitted[0];

	gaze_x = FCString::Atof(*splitted[5]);
	gaze_y = FCString::Atof(*splitted[6]);
	gaze_z = FCString::Atof(*splitted[7]);

	agv_x = FCString::Atof(*splitted[3]);
	agv_y = FCString::Atof(*splitted[4]);

	user_x = FCString::Atof(*splitted[1]);
	user_y = FCString::Atof(*splitted[2]);

	int _agv_num = FCString::Atoi(*agv_name.Mid(3, 2));
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("AGV name %s AGV number %d"), *agv_name, agv_num));
	//return;
	this->reset_flag = false;
	if (this->agv_num != _agv_num) this->reset_flag = true;
	this->agv_num = _agv_num;
	float yaw = 45.f;
	features->SetRawFeatures(user_x, user_y, agv_x, agv_y, gaze_x, gaze_y, gaze_z, yaw);
	//features->SetStationInfo(start_station, end_station);
}

void ADataReader::WriteOneLine(AFeatures* features, FString out_filename, bool print_to_screen)
{
	if (out_filename.Len() == 0) {
		out_filename = "PID";
		int _pid = FCString::Atoi(*this->PID);
		if (_pid < 10)
		{
			out_filename += "00" + this->PID;
		}
		else {
			out_filename += "0" + this->PID;
		}

		out_filename += "_" + this->SCN + ".csv";
	}

	FString save_file = FPaths::Combine(FPaths::ProjectDir(), "Data", "Output", "GeneratedFeatures", out_filename);
	this->header += LINE_TERMINATOR;
	if (!this->header_written)
	{
		FFileHelper::SaveStringToFile(this->header, *save_file, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_AllowRead);
		this->header_written = true;
	}
	FString data_string = "";
	
	//AGV_name
	data_string += FString::FromInt(this->agv_num) + ',';

	// User location
	data_string += FString::SanitizeFloat(features->user_location.X) + ",";
	data_string += FString::SanitizeFloat(features->user_location.Y) + ",";
	
	// AGV-User distance
	data_string += FString::SanitizeFloat(features->agv_to_user_distance.X) + ",";
	data_string += FString::SanitizeFloat(features->agv_to_user_distance.Y) + ",";

	// AGV Speed
	data_string += FString::SanitizeFloat(features->agv_speed.X) + ",";
	data_string += FString::SanitizeFloat(features->agv_speed.Y) + ",";
	data_string += FString::SanitizeFloat(features->agv_speed.Length()) + ",";

	// User speed
	data_string += FString::SanitizeFloat(abs(features->user_velocity.X)) + ",";
	data_string += FString::SanitizeFloat(abs(features->user_velocity.Y)) + ",";
	data_string += FString::SanitizeFloat(features->user_velocity.Length()) + ",";


	// User velocity
	data_string += FString::SanitizeFloat(features->user_velocity.X) + ",";
	data_string += FString::SanitizeFloat(features->user_velocity.Y) + ",";

	// Wait time
	data_string += FString::SanitizeFloat(features->wait_time) + ",";

	// Intent to cross
	data_string += features->intent_to_cross ? "TRUE," : "FALSE,";

	// Gazing Station
	data_string += FString::FromInt(features->gazing_station) + ",";

	// Possible interaction
	data_string += features->possible_interaction ? "TRUE," : "FALSE,";

	// Facing sidewalk
	data_string += features->facing_sidewalk ? "TRUE," : "FALSE,";

	// Facing road
	data_string += features->facing_road ? "TRUE," : "FALSE,";

	// On sidewalk
	data_string += features->on_sidewalk ? "TRUE," : "FALSE,";

	// On road
	data_string += features->on_road ? "TRUE," : "FALSE,";

	// closest station
	data_string += FString::FromInt(features->closest_station) + ",";
	data_string += FString::SanitizeFloat(features->distance_to_closest_station.Length()) + ",";
	data_string += FString::SanitizeFloat(features->distance_to_closest_station.X) + ",";
	data_string += FString::SanitizeFloat(features->distance_to_closest_station.Y) + ",";

	// Looking at AGV
	data_string += features->looking_at_agv ? "TRUE," : "FALSE,";

	// Gaze direction
	data_string += FString::SanitizeFloat(features->gaze_vector_3d.X) + ",";
	data_string += FString::SanitizeFloat(features->gaze_vector_3d.Y) + ",";
	data_string += FString::SanitizeFloat(features->gaze_vector_3d.Z) + ",";

	// AGV location
	data_string += FString::SanitizeFloat(features->agv_location.X) + ",";
	data_string += FString::SanitizeFloat(features->agv_location.Y) + ",";

	// Looking at closest station
	data_string += features->looking_at_closest_station ? "TRUE" : "FALSE";

	data_string += LINE_TERMINATOR;
	FFileHelper::SaveStringToFile(data_string, *save_file, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_Append);
}

void ADataReader::WriteState(FString state, FString out_filename)
{
	if (out_filename.Len() == 0) {
		out_filename = "PID";
		int _pid = FCString::Atoi(*this->PID);
		if (_pid < 10)
		{
			out_filename += "00" + this->PID;
		}
		else {
			out_filename += "0" + this->PID;
		}

		out_filename += "_" + this->SCN + "_States.csv";
	}
	FString save_file = FPaths::Combine(FPaths::ProjectDir(), "Data", "Output", "StatePredictions", out_filename);

	if (!this->state_header_written)
	{
		FString state_header = "State";
		state_header += LINE_TERMINATOR;
		FFileHelper::SaveStringToFile(state_header, *save_file, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_Append);
		this->state_header_written = true;
	}
	state += LINE_TERMINATOR;
	FFileHelper::SaveStringToFile(state, *save_file, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), EFileWrite::FILEWRITE_Append);
}
