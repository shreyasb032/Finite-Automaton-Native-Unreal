// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FutureCollisionChecker.generated.h"

UCLASS()
class FINITEAUTOMATONMODEL_API AFutureCollisionChecker : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFutureCollisionChecker();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float yellowBoxWidth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float yellowBoxHeight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int frameStep;

	UFUNCTION(BlueprintCallable)
	bool CheckFutureCollisions(const TArray<FVector2D>& PredictedWorkerPositions, const TArray<FVector2D>& FutureAGVPositions, const TArray<FVector2D>& FutureAGVHeading) const;

};
