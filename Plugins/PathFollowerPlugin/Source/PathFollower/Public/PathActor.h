#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "PathActor.generated.h"

UCLASS()
class PATHFOLLOWER_API APathActor : public AActor
{
	GENERATED_BODY()
	
public:	
	APathActor();

protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	TObjectPtr<USplineComponent> PathSpline;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> PathMesh;

public:	
	// Get spline component
	TObjectPtr<USplineComponent> GetPathSpline() const;
};
