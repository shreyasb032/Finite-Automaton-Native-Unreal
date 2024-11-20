// Fill out your copyright notice in the Description page of Project Settings.


#include "FutureCollisionChecker.h"

// Sets default values
AFutureCollisionChecker::AFutureCollisionChecker()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AFutureCollisionChecker::BeginPlay()
{
	Super::BeginPlay();
	
}

bool AFutureCollisionChecker::CheckFutureCollisions(const TArray<FVector2D>& PredictedWorkerPositions, const TArray<FVector2D>& FutureAGVPositions,
													const TArray<FVector2D>& FutureAGVHeading) const
{
	for (int i = 0; i < PredictedWorkerPositions.Num(); i += frameStep)
	{
		FVector2D heading = FutureAGVHeading[i];
		FVector2D heading_perp = heading.GetRotated(90.0);
		FVector2D topLeft = FutureAGVPositions[i] + heading * yellowBoxHeight / 2 + heading_perp * yellowBoxWidth / 2;
		FVector2D bottomRight = FutureAGVPositions[i] - heading * yellowBoxHeight / 2 - heading_perp * yellowBoxWidth / 2;
		FBox2D yellowZone = FBox2D(bottomRight, topLeft);
		if (yellowZone.IsInside(PredictedWorkerPositions[i]))
		{
			return true;
		}
	}
	return false;
}
