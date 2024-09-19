// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Features.h"
#include "Constants.h"
#include "GameFramework/Actor.h"
#include "FiniteAutomaton.generated.h"

UCLASS(Blueprintable)
class FINITEAUTOMATONMODEL_API AFiniteAutomaton : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFiniteAutomaton();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	//virtual void Tick(float DeltaTime) override;
	UFUNCTION(BlueprintCallable)
	FString GetState(AFeatures* current);
	FString current_state_name;
	int current_state_id;
	int prev_state_id;
	TArray<bool> error_flags;
	TMap<int, FString> idx_to_name;
	
	// The MLE probabilities of transition
	TMap<int, TArray<float>> MLE;

	// function to reset it for each new AGV
	UFUNCTION(BlueprintCallable)
	bool reset();


private:
	Constants constants;

	// State constraint checkers
	bool AtStationStateChecker(AFeatures* current) const;
	bool WaitingStateChecker(AFeatures* current);
	bool CrossingStateChecker(AFeatures* current);
	bool ApproachingSidewalkStateChecker(AFeatures* current) const;
	bool MovingAlongSidewalkStateChecker(AFeatures* current) const;
	bool ApproachingStationStateChecker(AFeatures* current) const;

	// Transition models
	int HandleErrorState(AFeatures* current);
	int HandleAtStationState(AFeatures* current) const;
	int HandleApproachSidewalkState(AFeatures* current) const;
	int HandleWaitState(AFeatures* current) const;
	int HandleMovingAlongSidewalkState(AFeatures* current) const;
	int HandleCrossState(AFeatures* current) const;
	int HandleApproachStationState(AFeatures* current) const;
	
	int ConstraintChecking(AFeatures* current);
};
