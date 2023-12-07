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
	FString GetState(AFeatures* current);
	FString CurrentState;
	TArray<bool> error_flags;

	// function to reset it for each new AGV
	UFUNCTION(Blueprintable)
	bool reset();


private:
	Constants constants;

	// State constraint checkers
	bool StartStateChecker(AFeatures* current);
	bool WaitingStateChecker(AFeatures* current);
	bool CrossingStateChecker(AFeatures* current);
	bool ApproachingSidewalkStateChecker(AFeatures* current);
	bool MovingAlongSidewalkStateChecker(AFeatures* current);
	bool ApproachingStationStateChecker(AFeatures* current);
	bool ArrivedStateChecker(AFeatures* current);

	// Transition models
	FString HandleErrorState(AFeatures* current);
	FString HandleStartState(AFeatures* current);
	FString HandleApproachSidewalkState(AFeatures* current);
	FString HandleWaitState(AFeatures* current);
	FString HandleMovingAlongSidewalkState(AFeatures* current);
	FString HandleCrossState(AFeatures* current);
	FString HandleApproachStationState(AFeatures* current);
	FString HandleArrivedState(AFeatures* current);

	FString ConstraintChecking(FString current_state, AFeatures* current);
};
