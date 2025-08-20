#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
class USplineComponent;
class UChildActorComponent;
#include "PathActor.generated.h"

UCLASS()
class PATHFOLLOWER_API APathActor : public AActor
{
	GENERATED_BODY()
	
public:	
	APathActor();

    // C++専用: このアクターのスプラインコンポーネントを取得
    TObjectPtr<USplineComponent> GetSplineComponent() const { return PathSpline; }

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	// Properties to be set by derived classes in constructor
	float PathMarkerDistance = 500.0f;  // Distance between path markers
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Path")
    TSubclassOf<AActor> PathMarkerClass = nullptr;  // Class of actor to spawn as path markers

private:
    void RebuildPathMarkers();
    // Function to spawn path markers along the spline
    void SpawnPathMarkersAlongSpline();
    
    // Function to clear existing path markers
    void ClearPathMarkers();
	
private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USplineComponent> PathSpline;

	// Array to keep track of spawned path marker components for cleanup
	UPROPERTY()
	TArray<TObjectPtr<UChildActorComponent>> SpawnedPathMarkerComponents;
};
