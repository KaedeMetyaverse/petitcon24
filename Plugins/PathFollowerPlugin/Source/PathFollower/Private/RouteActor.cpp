#include "RouteActor.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"

ARouteActor::ARouteActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create and set up the spline component
	RouteSpline = CreateDefaultSubobject<USplineComponent>(TEXT("RouteSpline"));
	RootComponent = RouteSpline;
	
	// Configure spline properties
	RouteSpline->SetClosedLoop(false);
	RouteSpline->ClearSplinePoints();
	
	// Add default spline points
	RouteSpline->AddSplinePoint(FVector(0.0f, 0.0f, 0.0f), ESplineCoordinateSpace::Local);
	RouteSpline->AddSplinePoint(FVector(500.0f, 0.0f, 0.0f), ESplineCoordinateSpace::Local);
	RouteSpline->UpdateSpline();

	// Create a simple mesh component for visualization (optional)
	RouteMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RouteMesh"));
	RouteMesh->SetupAttachment(RootComponent);
	RouteMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ARouteActor::BeginPlay()
{
	Super::BeginPlay();
	
	// Additional initialization if needed
}
