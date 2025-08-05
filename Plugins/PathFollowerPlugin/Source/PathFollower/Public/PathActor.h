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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Path", meta = (AllowPrivateAccess = "true"))
	class USplineComponent* PathSpline;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Path", meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* PathMesh;

public:	
	// Get spline component
	FORCEINLINE USplineComponent* GetPathSpline() const { return PathSpline; }
};