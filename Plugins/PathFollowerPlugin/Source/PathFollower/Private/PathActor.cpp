#include "PathActor.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"

APathActor::APathActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create and set up the spline component
	PathSpline = CreateDefaultSubobject<USplineComponent>(TEXT("PathSpline"));
	RootComponent = PathSpline;
}

void APathActor::BeginPlay()
{
	Super::BeginPlay();
}

TObjectPtr<USplineComponent> APathActor::GetPathSpline() const {
	return PathSpline;
}
