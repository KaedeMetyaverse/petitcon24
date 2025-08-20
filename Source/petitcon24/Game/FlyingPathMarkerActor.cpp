#include "FlyingPathMarkerActor.h"
#include "TimerManager.h"
#include "GameFramework/Pawn.h"

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

	// プレイヤーが現在操作している Pawn との Overlap のみ反応
	APawn* OtherPawn = Cast<APawn>(OtherActor);
	if (nullptr == OtherPawn || !OtherPawn->IsPlayerControlled())
	{
		return;
	}

	// BPへ通知（実装されていれば呼び出し）
	OnOverlappedByPlayerPawn(OtherPawn);

	bDestroyScheduled = true;

	FTimerDelegate Delegate;
	Delegate.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(AFlyingPathMarkerActor, DestroySelf));
	GetWorldTimerManager().SetTimerForNextTick(Delegate);
}

void AFlyingPathMarkerActor::DestroySelf()
{
	Destroy();
}
