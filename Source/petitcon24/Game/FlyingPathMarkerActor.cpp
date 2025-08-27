#include "FlyingPathMarkerActor.h"
#include "TimerManager.h"
#include "GameFramework/Pawn.h"
#include "FlyingPawnCollisionActor.h"

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

	// AFlyingPawnCollisionActor 以外は無視
	if (!OtherActor->IsA(AFlyingPawnCollisionActor::StaticClass()))
	{
		return;
	}

	// プレイヤーが操作中 以外は無視
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (!PlayerController)
	{
		return;
	}

	if (!PlayerController->GetPawn())
	{
		return;
	}

	// BPへ通知（実装されていれば呼び出し）
	OnOverlappedByPlayerPawn();

	bDestroyScheduled = true;

	FTimerDelegate Delegate;
	Delegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(AFlyingPathMarkerActor, DestroySelf));
	GetWorldTimerManager().SetTimerForNextTick(Delegate);
}

void AFlyingPathMarkerActor::DestroySelf()
{
	Destroy();
}
