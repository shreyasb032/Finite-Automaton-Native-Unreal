// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "NNE.h"
#include "NNERuntimeCPU.h"
#include "NNEModelData.h"

#include "NNEInferenceActor.generated.h"

class FMyModelHelper
{
public:
	TUniquePtr<UE::NNE::IModelInstanceCPU> ModelInstance;
	TArray<float> InputData;
	TArray<float> OutputData;
	TArray<UE::NNE::FTensorBindingCPU> InputBindings;
	TArray<UE::NNE::FTensorBindingCPU> OutputBindings;
	bool bIsRunning;
};

UCLASS()
class FINITEAUTOMATONMODEL_API ANNEInferenceActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANNEInferenceActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UNNEModelData> PreLoadedModelData;

	UFUNCTION(BlueprintCallable)
	void RunModel(const TArray<float>& InputData, TArray<float>& OutputData);

	TArray<FVector2D> ConvertToVectors(const TArray<float>& OutputData);

private:
	TSharedPtr<FMyModelHelper> ModelHelper;

};
