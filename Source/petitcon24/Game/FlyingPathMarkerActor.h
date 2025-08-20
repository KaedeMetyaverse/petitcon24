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

private:
	// Overlap 時に再生する SE（エディタから設定）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USoundBase> OverlapSound = nullptr;

	// 多重スケジュール防止
	bool bDestroyScheduled = false;

	UFUNCTION()
	void DestroySelf();
};
