#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "Components/ChildActorComponent.h"
#include "PathActor.generated.h"

UCLASS()
class PATHFOLLOWER_API APathActor : public AActor
{
	GENERATED_BODY()
	
public:	
	APathActor();

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	// Properties to be set by derived classes in constructor
	float PathMarkerDistance = 500.0f;  // Distance between path markers
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Path")
    TSubclassOf<AActor> PathMarkerClass = nullptr;  // Class of actor to spawn as path markers

private:
	// Function to spawn path markers along the spline
	void SpawnPathMarkersAlongSpline();
	
	// Function to clear existing path markers
	void ClearPathMarkers();
	
private:
	UPROPERTY()
	TObjectPtr<USplineComponent> PathSpline;

	// Array to keep track of spawned path marker components for cleanup
	UPROPERTY()
	TArray<TObjectPtr<UChildActorComponent>> SpawnedPathMarkerComponents;
};
