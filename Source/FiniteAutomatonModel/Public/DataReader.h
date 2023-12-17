// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Features.h"
#include "GameFramework/Actor.h"
#include "DataReader.generated.h"

UCLASS()
class FINITEAUTOMATONMODEL_API ADataReader : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADataReader();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	// virtual void Tick(float DeltaTime) override;
	UPROPERTY(BlueprintReadWrite)
	FString PID;
	
	UPROPERTY(BlueprintReadWrite)
	FString SCN;

	UFUNCTION(BlueprintCallable)
	bool ReadData();

	UFUNCTION(BlueprintCallable)
	void GetOneLine(AFeatures* features, int line_num);

	UFUNCTION(BlueprintCallable)
	void WriteOneLine(AFeatures* features, FString out_filename, bool print_to_screen);

	UFUNCTION(BlueprintCallable)
	void WriteState(FString state, FString out_filename);

	UPROPERTY(BlueprintReadOnly)
	bool reset_flag;
	
	UPROPERTY(BlueprintReadOnly)
	int agv_num;


private:
	FString filename;
	TArray<FString> data;
	FString header;
	int line_number;
	int num_lines;
	TMap<int, int> agv_to_start;
	TMap<int, int> agv_to_end;
	bool header_written;
	bool state_header_written;
};
