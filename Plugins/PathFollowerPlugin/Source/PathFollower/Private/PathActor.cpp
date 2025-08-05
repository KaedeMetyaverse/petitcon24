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
	
	// Configure spline properties
	PathSpline->SetClosedLoop(false);
	PathSpline->ClearSplinePoints();
	
	// Add default spline points
	PathSpline->AddSplinePoint(FVector(0.0f, 0.0f, 0.0f), ESplineCoordinateSpace::Local);
	PathSpline->AddSplinePoint(FVector(500.0f, 0.0f, 0.0f), ESplineCoordinateSpace::Local);
	PathSpline->UpdateSpline();

	// Create a simple mesh component for visualization (optional)
	PathMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PathMesh"));
	PathMesh->SetupAttachment(RootComponent);
	PathMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void APathActor::BeginPlay()
{
	Super::BeginPlay();
	
	// Additional initialization if needed
}

TObjectPtr<USplineComponent> APathActor::GetPathSpline() const {
	return PathSpline;
}
