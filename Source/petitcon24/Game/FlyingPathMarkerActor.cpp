#include "FlyingPathMarkerActor.h"
#include "TimerManager.h"

AFlyingPathMarkerActor::AFlyingPathMarkerActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AFlyingPathMarkerActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (bDestroyScheduled)
	{
		return;
	}

	bDestroyScheduled = true;

	FTimerDelegate Delegate;
	Delegate.BindUFunction(this, FName("DestroySelf"));
	GetWorldTimerManager().SetTimerForNextTick(Delegate);
}

void AFlyingPathMarkerActor::DestroySelf()
{
	Destroy();
}
