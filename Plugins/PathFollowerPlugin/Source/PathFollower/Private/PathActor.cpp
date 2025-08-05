#include "PathActor.h"
#include "Components/SplineComponent.h"
#include "Components/ChildActorComponent.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY_STATIC(LogPathActor, Log, All);

APathActor::APathActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create and set up the spline component
	PathSpline = CreateDefaultSubobject<USplineComponent>(TEXT("PathSpline"));
	RootComponent = PathSpline;
	
#if WITH_EDITORONLY_DATA
	bIsInEditor = true;
#endif
}

void APathActor::BeginPlay()
{
	Super::BeginPlay();
	
#if WITH_EDITORONLY_DATA
	bIsInEditor = false;
#endif
	
	// Clear any editor-spawned markers and spawn runtime markers
	ClearPathMarkers();
	SpawnPathMarkersAlongSpline();
}

void APathActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	// Clear existing markers and spawn new ones
	ClearPathMarkers();
	SpawnPathMarkersAlongSpline();
}

#if WITH_EDITOR
void APathActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	// Refresh path markers when any property changes (mainly spline changes)
	ClearPathMarkers();
	SpawnPathMarkersAlongSpline();
}
#endif

void APathActor::ClearPathMarkers()
{
	// Remove all previously spawned path marker components
	for (TObjectPtr<UChildActorComponent>& MarkerComponent : SpawnedPathMarkerComponents)
	{
		if (IsValid(MarkerComponent))
		{
			MarkerComponent->DestroyComponent();
		}
	}
	
	SpawnedPathMarkerComponents.Empty();
}

void APathActor::SpawnPathMarkersAlongSpline()
{
	// Validate configuration before spawning
	if (nullptr == PathMarkerClass)
	{
		UE_LOG(LogPathActor, Error, TEXT("PathActor '%s': PathMarkerClass is not set. No path markers will be spawned."), *GetName());
		return;
	}
	
	if (PathMarkerDistance <= 0.0f)
	{
		UE_LOG(LogPathActor, Error, TEXT("PathActor '%s': PathMarkerDistance is invalid (%.2f). Must be greater than 0. No path markers will be spawned."), *GetName(), PathMarkerDistance);
		return;
	}
	
	check(PathSpline != nullptr);
	
	// Get the total length of the spline
	const auto SplineLength = PathSpline->GetSplineLength();

	UE_LOG(LogPathActor, Verbose, TEXT("PathActor '%s': Starting to spawn path markers. SplineLength=%.2f, PathMarkerDistance=%.2f, MarkerClass=%s"), 
		*GetName(), SplineLength, PathMarkerDistance, *PathMarkerClass->GetName());
	
	// Calculate number of path markers to spawn
	int32 NumMarkersToSpawn = (FMath::CeilToInt(SplineLength / PathMarkerDistance) - 1) + 2;
	
	UE_LOG(LogPathActor, Verbose, TEXT("PathActor '%s': Calculated %d path markers to spawn along spline"), *GetName(), NumMarkersToSpawn);

	// Create path marker components at intervals along the spline
	for (int32 i = 0; i < NumMarkersToSpawn; ++i)
	{
		const float DesiredDistance = i * PathMarkerDistance;
		const float Distance = FMath::Min(DesiredDistance, SplineLength);
		
		// Get location and rotation at this distance along the spline
		const FVector SpawnLocation = PathSpline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
		const FRotator SpawnRotation = PathSpline->GetRotationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);

		// Create a new child actor component for the path marker
		FString ComponentName = FString::Printf(TEXT("PathMarker_%d"), i);
		TObjectPtr<UChildActorComponent> MarkerComponent = NewObject<UChildActorComponent>(this, *ComponentName);
		
		// Set up the component
		MarkerComponent->SetChildActorClass(PathMarkerClass);
		MarkerComponent->SetWorldLocationAndRotation(SpawnLocation, SpawnRotation);
		
		// Attach to the root component
		MarkerComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
		
		// Register the component
		MarkerComponent->RegisterComponent();
		
		// Create the child actor
		MarkerComponent->CreateChildActor();
		
		// Add to our tracking array
		SpawnedPathMarkerComponents.Add(MarkerComponent);
		
		UE_LOG(LogPathActor, Verbose, TEXT("PathActor '%s': Successfully created path marker component %d at distance %.2f, location %s"), 
			*GetName(), i, Distance, *SpawnLocation.ToString());
	}
}
