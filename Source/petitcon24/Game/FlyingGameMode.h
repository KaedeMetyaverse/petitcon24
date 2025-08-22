#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "UObject/SoftObjectPtr.h"
#include "InGameInfoViewModel.h"
#include "TimerManager.h"
#include "FlyingGameMode.generated.h"

class AFlyingPlayerController;
class APathActor;
class ULevelSequence;
class ALevelSequenceActor;
class ULevelSequencePlayer;
class ULoadingOverlayBase;
class UUserWidget;
class UInGameInfoSubsystem;
class ULevel;

UCLASS(abstract)
class PETITCON24_API AFlyingGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFlyingGameMode();

    virtual void BeginPlay() override;

private:
    // UI Z-order and timing constants
    static constexpr int32 InGameInfoZOrder = 5000;
    static constexpr int32 HowToZOrder = 6000;
    static constexpr int32 LoadingOverlayZOrder = 1000;
    static constexpr float FadeTickSeconds = 0.01f;

    // Latent action UUIDs
    static constexpr int32 UUID_OnStageUnloaded = 1001;
    static constexpr int32 UUID_OnStageLoaded = 1002;

    // オープニング用レベルシーケンスを再生する
    void PlayOpeningSequence();

    // シーケンス開始
    void StartSequence();

    // 次の PathActor のスプラインへ移動開始。範囲外なら何もしない
    void TryStartNextPath();

    // エンディング用レベルシーケンスを再生する
    void PlayEndingSequence();

    // 共通: レベルシーケンスから Player/Actor を生成（未設定時は警告して false）
    bool CreateSequencePlayer(
        const TSoftObjectPtr<ULevelSequence>& SequencePtr,
        TObjectPtr<ULevelSequencePlayer>& OutPlayer,
        TObjectPtr<ALevelSequenceActor>& OutActor,
        const FText& NotSetMessage);

    // 現在のステージに対して移動を開始する（既にロード済みのレベルを受け取る）
    void StartMovementForCurrentStage(ULevel* LoadedLevel);

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

    // オープニングシーケンスの再生完了
    UFUNCTION()
    void HandlePlayOpeningSequenceFinished();

    // エンディングシーケンスの再生完了
    UFUNCTION()
    void HandlePlayEndingSequenceFinished();

    // Persistent Level 内でタグ一致の Pawn を探す
    APawn* FindPawnByTagInPersistentLevel(FName Tag) const;

private:
    TObjectPtr<AFlyingPlayerController> CachedFlyingPlayerController;
    int32 CurrentPathIndex = INDEX_NONE;

    // 現在ロード済みのレベルをキャッシュ
    UPROPERTY()
    TObjectPtr<ULevel> CurrentLoadedLevel;
    
    // ステージ（レベル）の順序配列
    UPROPERTY(EditDefaultsOnly, Category = "Game")
    TArray<TSoftObjectPtr<UWorld>> Stages;

    // ゲーム開始時に再生するレベルシーケンス
    UPROPERTY(EditDefaultsOnly, Category = "Game")
    TSoftObjectPtr<ULevelSequence> OpeningSequence;

    // Opening 後に Possess する対象のタグ
    UPROPERTY(EditDefaultsOnly, Category = "Game")
    FName FlyingPawnTag;

    // 再生用に保持（GC対策）
    UPROPERTY()
    TObjectPtr<ULevelSequencePlayer> OpeningSequencePlayer;

    UPROPERTY()
    TObjectPtr<ALevelSequenceActor> OpeningSequenceActor;

    // 全行程完了時に再生するレベルシーケンス
    UPROPERTY(EditDefaultsOnly, Category = "Game")
    TSoftObjectPtr<ULevelSequence> EndingSequence;

    // 再生用に保持（GC対策）
    UPROPERTY()
    TObjectPtr<ULevelSequencePlayer> EndingSequencePlayer;

    UPROPERTY()
    TObjectPtr<ALevelSequenceActor> EndingSequenceActor;

    // ローディング動画オーバーレイ関連
    UPROPERTY(EditDefaultsOnly, Category = "LoadingOverlay")
    TSubclassOf<ULoadingOverlayBase> LoadingOverlayClass;

    UPROPERTY()
    TObjectPtr<ULoadingOverlayBase> LoadingOverlayWidget;

    UPROPERTY(EditDefaultsOnly, Category = "LoadingOverlay")
    float LoadingMinDurationSeconds = 5.0f;

    UPROPERTY(EditDefaultsOnly, Category = "LoadingOverlay")
    float FadeInDurationSeconds = 1.5f;

    UPROPERTY(EditDefaultsOnly, Category = "LoadingOverlay")
    float FadeOutDurationSeconds = 1.5f;

    // フェード・状態管理
    bool bIsOverlayVisible = false;
    bool bMinDurationSatisfied = false;
    bool bNextStageReady = false;
    bool bPendingUnloadAfterFadeIn = false;

    float FadeElapsedSeconds = 0.f;
    float FadeTotalDuration = 0.f;
    float FadeFromOpacity = 0.f;
    float FadeToOpacity = 1.f;

    FTimerHandle FadeTimerHandle;
    FTimerHandle MinDurationTimerHandle;

    void BeginLoadingOverlayBeforeTransition();
    void StartFade(float FromOpacity, float ToOpacity, float Duration, bool bIsFadeIn);
    void TickFade();
    void OnFadeInFinished();
    void OnFadeOutFinished();
    void OnMinDurationReached();
    void TryFinishTransitionAfterLoadAndMin();

    // ゲーム中に表示する情報ウィジェット（エディタ設定可能）
    UPROPERTY(EditDefaultsOnly, Category = "InGameUI")
    TSubclassOf<UUserWidget> InGameInfoWidgetClass;

    // 生成済みインスタンス（GC保護）
    UPROPERTY()
    TObjectPtr<UUserWidget> InGameInfoWidget;

    void ShowInGameWidget();
    void HideInGameWidget();

    // ゲーム開始直後の操作説明UI（一定秒数のみ表示）
    UPROPERTY(EditDefaultsOnly, Category = "InGameUI")
    TSubclassOf<UUserWidget> InGameHowToWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "InGameUI")
    float HowToWidgetDurationSeconds = 20.0f;

    UPROPERTY()
    TObjectPtr<UUserWidget> InGameHowToWidget;

    bool bHasShownHowToWidget = false;

    FTimerHandle HowToWidgetTimerHandle;

    void ShowHowToWidget();
    void HideHowToWidget();

    // MVVM: In-Game 情報表示用 ViewModel は LocalPlayerSubsystem で管理
    void UpdateViewModelStageState();

    // 一度だけ取得して保持するローカルプレイヤー用サブシステム
    UPROPERTY()
    TObjectPtr<UInGameInfoSubsystem> InGameInfoSubsystem;
};
