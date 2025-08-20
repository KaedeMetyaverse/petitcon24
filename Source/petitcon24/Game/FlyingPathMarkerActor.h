#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FlyingPathMarkerActor.generated.h"

UCLASS()
class PETITCON24_API AFlyingPathMarkerActor : public AActor
{
	GENERATED_BODY()

public:
	AFlyingPathMarkerActor();

protected:
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	// プレイヤー操作中のPawnとOverlapした時にBPで処理したい場合に実装
	UFUNCTION(BlueprintImplementableEvent, Category="Events")
	void OnOverlappedByPlayerPawn(APawn* PlayerPawn);

private:
	// 多重スケジュール防止
	bool bDestroyScheduled = false;

	UFUNCTION()
	void DestroySelf();
};
