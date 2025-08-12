#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/Actor.h"
#include "FlyingPlayerController.h"
#include "PathActor.h"
#include "PathSequenceInfo.h"
#include "FlyingGameMode.generated.h"

UCLASS()
class PETITCON24_API AFlyingGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFlyingGameMode();

    virtual void BeginPlay() override;

private:
    // シーケンス開始
    void StartSequence();

    // 次の PathActor のスプラインへ移動開始。範囲外なら何もしない
    void TryStartNextPath();

    // コールバック: 1本のスプライン完了時
    void HandleMoveAlongSplineFinished();

private:
    TObjectPtr<AFlyingPlayerController> CachedFlyingPlayerController;
    int32 CurrentPathIndex = INDEX_NONE;
    TArray<TObjectPtr<APathActor>> PathActorsSequence;
};
