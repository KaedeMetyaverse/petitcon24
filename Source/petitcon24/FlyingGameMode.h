#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/Actor.h"
#include "FlyingPlayerController.h"
#include "PathActor.h"
#include "UObject/SoftObjectPtr.h"
#include "FlyingGameMode.generated.h"

UCLASS(abstract)
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

    // 指定レベル内の唯一の APathActor を取得（見つからない/複数あるなら nullptr）
    APathActor* FindUniquePathActorInStreamingLevel(ULevel* Level) const;

    // 非ブロッキングのステージ切替フロー
    void ProceedUnloadPreviousStage();
    void ProceedLoadCurrentStage();

    UFUNCTION()
    void OnStageUnloaded();

    UFUNCTION()
    void OnStageLoaded();

private:
    TObjectPtr<AFlyingPlayerController> CachedFlyingPlayerController;
    int32 CurrentPathIndex = INDEX_NONE;
    
    // ステージ（レベル）の順序配列
    UPROPERTY(EditDefaultsOnly, Category = "Stages")
    TArray<TSoftObjectPtr<UWorld>> Stages;
};
