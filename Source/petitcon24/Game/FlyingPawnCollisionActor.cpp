#include "FlyingPawnCollisionActor.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SphereComponent.h"

AFlyingPawnCollisionActor::AFlyingPawnCollisionActor()
{
    SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
    RootComponent = SphereComponent;

    // Tick 不要
    PrimaryActorTick.bCanEverTick = false;
}

USphereComponent* AFlyingPawnCollisionActor::GetSphereComponent() const
{
    return SphereComponent;
}
