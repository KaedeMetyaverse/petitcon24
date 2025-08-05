#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "RouteActor.generated.h"

UCLASS()
class PATHFOLLOWER_API ARouteActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ARouteActor();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Route", meta = (AllowPrivateAccess = "true"))
	class USplineComponent* RouteSpline;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Route", meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* RouteMesh;

public:	
	// Get spline component
	FORCEINLINE USplineComponent* GetRouteSpline() const { return RouteSpline; }
};
