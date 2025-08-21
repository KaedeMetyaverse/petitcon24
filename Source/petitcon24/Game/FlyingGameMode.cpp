#include "FlyingGameMode.h"
#include "FlyingPlayerController.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Actor.h"
#include "EngineUtils.h"
#include "Logging/LogMacros.h"
#include "FlyingPawn.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Modules/ModuleManager.h"
#include "Internationalization/Text.h"
#include "Engine/LevelStreaming.h"
#include "Engine/Level.h"
#include "Kismet/GameplayStatics.h"
#include "InGameInfoSubsystem.h"
#include "PathActor.h"
#include "LoadingOverlayBase.h"
#include "Blueprint/UserWidget.h"
#if WITH_EDITOR
#include "Logging/MessageLog.h"
#endif

DEFINE_LOG_CATEGORY_STATIC(LogFlyingGameMode, Log, All);

#define LOCTEXT_NAMESPACE "FlyingGameMode"
AFlyingGameMode::AFlyingGameMode()
{
}

void AFlyingGameMode::BeginPlay()
{
    Super::BeginPlay();

    // InGameInfoSubsystem をキャッシュし、StageCount を初期設定
    {
        UWorld* World = GetWorld();
        check(World != nullptr);
        APlayerController* PC = World->GetFirstPlayerController();
        check(PC != nullptr);
        ULocalPlayer* LP = PC->GetLocalPlayer();
        check(LP != nullptr);
        InGameInfoSubsystem = LP->GetSubsystem<UInGameInfoSubsystem>();
        check(InGameInfoSubsystem != nullptr);

        // ステージ総数は不変なので最初の一回のみ設定
        InGameInfoSubsystem->SetStageCount(Stages.Num());
    }

    // 既存のステージ進行フローを開始
    StartSequence();
}

void AFlyingGameMode::ShowInGameWidget()
{
    if (!InGameInfoWidgetClass)
    {
#if WITH_EDITOR
        if (FModuleManager::Get().IsModuleLoaded("MessageLog"))
        {
            FMessageLog Log("PIE");
            Log.Warning(LOCTEXT("InGameWidgetClassNotSet", "InGameInfoWidgetClass is not set in GameMode. HUD will not be shown."));
        }
#endif
        UE_LOG(LogFlyingGameMode, Warning, TEXT("InGameInfoWidgetClass is not set in GameMode. HUD will not be shown."));
        return;
    }

    if (!InGameInfoWidget)
    {
        UWorld* World = GetWorld();
        check(World != nullptr);

        InGameInfoWidget = CreateWidget<UUserWidget>(World, InGameInfoWidgetClass);
        check(InGameInfoWidget != nullptr);

        InGameInfoWidget->AddToViewport(/*ZOrder*/ InGameInfoZOrder);
    }
    else if (!InGameInfoWidget->IsInViewport())
    {
        InGameInfoWidget->AddToViewport(/*ZOrder*/ InGameInfoZOrder);
    }
}

void AFlyingGameMode::HideInGameWidget()
{
    if (InGameInfoWidget)
    {
        InGameInfoWidget->RemoveFromParent();
        InGameInfoWidget = nullptr;
    }
}

void AFlyingGameMode::ShowHowToWidget()
{
    if (bHasShownHowToWidget)
    {
        return;
    }

    if (!InGameHowToWidgetClass)
    {
#if WITH_EDITOR
        if (FModuleManager::Get().IsModuleLoaded("MessageLog"))
        {
            FMessageLog Log("PIE");
            Log.Warning(LOCTEXT("HowToWidgetClassNotSet", "InGameHowToWidgetClass is not set in GameMode. How-To UI will not be shown."));
        }
#endif
        UE_LOG(LogFlyingGameMode, Warning, TEXT("InGameHowToWidgetClass is not set in GameMode. How-To UI will not be shown."));
        bHasShownHowToWidget = true; // 再試行しない
        return;
    }

    UWorld* World = GetWorld();
    check(World != nullptr);

    if (!InGameHowToWidget)
    {
        InGameHowToWidget = CreateWidget<UUserWidget>(World, InGameHowToWidgetClass);
        check(InGameHowToWidget != nullptr);
    }

    if (!InGameHowToWidget->IsInViewport())
    {
        InGameHowToWidget->AddToViewport(/*ZOrder*/ HowToZOrder);
    }

    bHasShownHowToWidget = true;

    const float Duration = FMath::Max(0.0f, HowToWidgetDurationSeconds);
    if (Duration > 0.0f)
    {
        World->GetTimerManager().SetTimer(HowToWidgetTimerHandle, this, &AFlyingGameMode::HideHowToWidget, Duration, /*bLoop*/ false);
    }
    else
    {
        HideHowToWidget();
    }
}

void AFlyingGameMode::HideHowToWidget()
{
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(HowToWidgetTimerHandle);
    }

    if (InGameHowToWidget)
    {
        InGameHowToWidget->RemoveFromParent();
        InGameHowToWidget = nullptr;
    }
}

void AFlyingGameMode::PlayOpeningSequence()
{
    // 未設定なら即移動開始
    if (!CreateSequencePlayer(OpeningSequence, OpeningSequencePlayer, OpeningSequenceActor, LOCTEXT("OpeningSequenceNotSet", "OpeningSequence is not set in GameMode.")))
    {
        // Opening が無い場合でもゲーム中 HUD を表示
        ShowInGameWidget();
        // Opening が無い場合も操作説明UIを表示
        ShowHowToWidget();
        StartMovementForCurrentStage(CurrentLoadedLevel);
        return;
    }

    // 再生完了イベントにバインド
    OpeningSequencePlayer->OnFinished.AddDynamic(this, &AFlyingGameMode::HandlePlayOpeningSequenceFinished);
    OpeningSequencePlayer->Play();
}

void AFlyingGameMode::HandlePlayOpeningSequenceFinished()
{
    check(OpeningSequencePlayer != nullptr);
    OpeningSequencePlayer->OnFinished.RemoveDynamic(this, &AFlyingGameMode::HandlePlayOpeningSequenceFinished);

    // タグ未設定はログ出力して終了
    if (FlyingPawnTag.IsNone())
    {
#if WITH_EDITOR
        if (FModuleManager::Get().IsModuleLoaded("MessageLog"))
        {
            FMessageLog Log("PIE");
            Log.Error(LOCTEXT("OpeningPossessTagNotSet", "FlyingPawnTag is not set in GameMode."));
        }
#endif
        UE_LOG(LogFlyingGameMode, Error, TEXT("FlyingPawnTag is not set in GameMode."));
        return;
    }

    const UWorld* World = GetWorld();
    check(World != nullptr);

    // Opening 再生後に、Persistent Level 上の指定タグPawnをPossess
    APawn* TargetPawn = FindPawnByTagInPersistentLevel(FlyingPawnTag);
    if (nullptr == TargetPawn)
    {
#if WITH_EDITOR
        if (FModuleManager::Get().IsModuleLoaded("MessageLog"))
        {
            FMessageLog Log("PIE");
            Log.Error(FText::Format(LOCTEXT("OpeningPossessTargetNotFoundFmt", "Pawn with tag {0} not found in Persistent Level: {1}"), FText::FromName(FlyingPawnTag), FText::FromName(World->PersistentLevel->GetFName())));
        }
#endif
        UE_LOG(LogFlyingGameMode, Error, TEXT("Pawn with tag %s not found in Persistent Level: %s"), *FlyingPawnTag.ToString(), *World->PersistentLevel->GetName());
        return;
    }

    APlayerController* PC = World->GetFirstPlayerController();
    check(PC != nullptr);

    PC->UnPossess();
    PC->Possess(TargetPawn);

    // その後、移動開始（キャッシュを利用）
    check(CurrentLoadedLevel != nullptr);
    // Opening 終了後にゲーム中 HUD を表示
    ShowInGameWidget();
    // 操作説明UI（初回のみ、指定秒数表示）
    ShowHowToWidget();
    StartMovementForCurrentStage(CurrentLoadedLevel);
}

void AFlyingGameMode::StartSequence()
{
    UWorld* World = GetWorld();
    check(World != nullptr);

    // ステージ配列が空の場合はエラー
    if (Stages.Num() == 0)
    {
#if WITH_EDITOR
        if (FModuleManager::Get().IsModuleLoaded("MessageLog"))
        {
            FMessageLog Log("PIE");
            Log.Error(LOCTEXT("NoStagesSpecified", "No stages specified in GameMode. Set at least one stage (level)."));
        }
#endif
        UE_LOG(LogFlyingGameMode, Error, TEXT("No stages specified in GameMode. Set at least one stage (level)."));
        return;
    }

    APlayerController* PC = World->GetFirstPlayerController();
    AFlyingPlayerController* FlyingPC = Cast<AFlyingPlayerController>(PC);
    check(FlyingPC != nullptr);

    CachedFlyingPlayerController = FlyingPC;
    // 完了通知にバインド
    FlyingPC->OnMoveAlongSplineFinished().AddUObject(this, &AFlyingGameMode::HandleMoveAlongSplineFinished);

    CurrentPathIndex = -1;
    TryStartNextPath();
}

void AFlyingGameMode::UpdateViewModelStageState()
{
    check(InGameInfoSubsystem != nullptr);

    const int32 CurrentStageNumber = CurrentPathIndex + 1;

    InGameInfoSubsystem->SetCurrentStageNumber(CurrentStageNumber);
}

void AFlyingGameMode::PlayEndingSequence()
{
    // 未設定なら何もせず終了
    if (!CreateSequencePlayer(EndingSequence, EndingSequencePlayer, EndingSequenceActor, LOCTEXT("EndingSequenceNotSet", "EndingSequence is not set in GameMode.")))
    {
        return;
    }

    EndingSequencePlayer->OnFinished.AddDynamic(this, &AFlyingGameMode::HandlePlayEndingSequenceFinished);
    EndingSequencePlayer->Play();
}

void AFlyingGameMode::HandlePlayEndingSequenceFinished()
{
    if (EndingSequencePlayer)
    {
        EndingSequencePlayer->OnFinished.RemoveDynamic(this, &AFlyingGameMode::HandlePlayEndingSequenceFinished);
    }

    // 必要ならここでポストエンディングのフロー（例: メニュー遷移）を実装
}

bool AFlyingGameMode::CreateSequencePlayer(
    const TSoftObjectPtr<ULevelSequence>& SequencePtr,
    TObjectPtr<ULevelSequencePlayer>& OutPlayer,
    TObjectPtr<ALevelSequenceActor>& OutActor,
    const FText& NotSetMessage)
{
    if (!SequencePtr.IsValid() && !SequencePtr.ToSoftObjectPath().IsValid())
    {
#if WITH_EDITOR
        if (FModuleManager::Get().IsModuleLoaded("MessageLog"))
        {
            FMessageLog Log("PIE");
            Log.Warning(NotSetMessage);
        }
#endif
        UE_LOG(LogFlyingGameMode, Warning, TEXT("%s"), *NotSetMessage.ToString());
        return false;
    }

    ULevelSequence* SequenceAsset = SequencePtr.LoadSynchronous();
    check(SequenceAsset != nullptr);

    ALevelSequenceActor* CreatedActor = nullptr;
    ULevelSequencePlayer* CreatedPlayer = ULevelSequencePlayer::CreateLevelSequencePlayer(this, SequenceAsset, FMovieSceneSequencePlaybackSettings(), CreatedActor);
    check(CreatedPlayer != nullptr);
    check(CreatedActor != nullptr);

    OutPlayer = CreatedPlayer;
    OutActor = CreatedActor;
    return true;
}

void AFlyingGameMode::TryStartNextPath()
{
    check(nullptr != CachedFlyingPlayerController);

    ++CurrentPathIndex;
    if (!Stages.IsValidIndex(CurrentPathIndex))
    {
        // 全行程完了: PlayerController から Pawn を UnPossess し、エンディングを再生
        const UWorld* World = GetWorld();
        check(World != nullptr);

        APlayerController* PC = World->GetFirstPlayerController();
        check(PC != nullptr);

        PC->UnPossess();
        // Ending 再生直前にゲーム中 HUD を非表示
        HideInGameWidget();
        PlayEndingSequence();

        return;
    }

    // 0番目（最初の）ステージではローディング画面を出さない
    if (CurrentPathIndex == 0)
    {
        // 最小表示時間は満了扱いにして、通常の非同期ロードへ
        bMinDurationSatisfied = true;
        ProceedUnloadPreviousStage();
        return;
    }

    // 以降のステージ切替前はローディングオーバーレイをフェードイン
    BeginLoadingOverlayBeforeTransition();
}

void AFlyingGameMode::HandleMoveAlongSplineFinished()
{
    // 次の PathActor のスプラインへ
    TryStartNextPath();
}

void AFlyingGameMode::ProceedUnloadPreviousStage()
{
    const int32 PreviousIndex = CurrentPathIndex - 1;
    if (PreviousIndex < 0)
    {
        ProceedLoadCurrentStage();
        return;
    }

    FLatentActionInfo LatentInfo;
    LatentInfo.CallbackTarget = this;
    LatentInfo.ExecutionFunction = GET_FUNCTION_NAME_CHECKED(AFlyingGameMode, OnStageUnloaded);
    LatentInfo.UUID = UUID_OnStageUnloaded;
    LatentInfo.Linkage = 0;
    UGameplayStatics::UnloadStreamLevelBySoftObjectPtr(this, Stages[PreviousIndex], LatentInfo, false /* non-blocking */);
}

void AFlyingGameMode::OnStageUnloaded()
{
    ProceedLoadCurrentStage();
}

void AFlyingGameMode::ProceedLoadCurrentStage()
{
    FLatentActionInfo LatentInfo;
    LatentInfo.CallbackTarget = this;
    LatentInfo.ExecutionFunction = GET_FUNCTION_NAME_CHECKED(AFlyingGameMode, OnStageLoaded);
    LatentInfo.UUID = UUID_OnStageLoaded;
    LatentInfo.Linkage = 0;
    UGameplayStatics::LoadStreamLevelBySoftObjectPtr(this, Stages[CurrentPathIndex], true /* visible */, false /* non-blocking */, LatentInfo);
}

void AFlyingGameMode::OnStageLoaded()
{
    const FString CurrLongPackageName = Stages[CurrentPathIndex].ToSoftObjectPath().GetLongPackageName();
    check(!CurrLongPackageName.IsEmpty());

    ULevelStreaming* Streaming = UGameplayStatics::GetStreamingLevel(this, FName(*CurrLongPackageName));
    if (!Streaming || !Streaming->IsLevelLoaded())
    {
#if WITH_EDITOR
        if (FModuleManager::Get().IsModuleLoaded("MessageLog"))
        {
            FMessageLog Log("PIE");
            Log.Error(FText::Format(
                LOCTEXT("StageNotInLevelsPanelFmt", "Streaming level not found or not loaded: {0}. Please add the stage to Window > Levels beforehand."),
                FText::FromString(CurrLongPackageName)));
        }
#endif
        UE_LOG(LogFlyingGameMode, Error, TEXT("Streaming level not found or not loaded: %s. Please add the stage to Window > Levels beforehand."), *CurrLongPackageName);
        return;
    }

    // キャッシュ更新
    CurrentLoadedLevel = Streaming->GetLoadedLevel();
    check(CurrentLoadedLevel != nullptr);

    // 次ステージのロード完了フラグ
    bNextStageReady = true;

    // Opening は 0 番目のステージロード時にだけ処理
    if (0 == CurrentPathIndex)
    {
        // ローディングオーバーレイが出ていた場合でも、Opening 再生前にフェードアウトを行う
        TryFinishTransitionAfterLoadAndMin();
        return;
    }

    // 通常フロー: フェードアウト完了を待ってから移動開始
    TryFinishTransitionAfterLoadAndMin();
}

void AFlyingGameMode::StartMovementForCurrentStage(ULevel* LoadedLevel)
{
    check(LoadedLevel != nullptr);

    APathActor* PathActor = FindUniquePathActorInStreamingLevel(LoadedLevel);
    if (nullptr == PathActor)
    {
        TryStartNextPath();
        return;
    }

    USplineComponent* SplineComp = PathActor->GetSplineComponent();
    check(SplineComp != nullptr);

    CachedFlyingPlayerController->StartMoveAlongSpline(SplineComp);
}

void AFlyingGameMode::BeginLoadingOverlayBeforeTransition()
{
    UWorld* World = GetWorld();
    check(World != nullptr);

    bMinDurationSatisfied = false;
    bNextStageReady = false;

    if (!LoadingOverlayClass)
    {
        // オーバーレイ未設定: MessageLog + 通常ログ
#if WITH_EDITOR
        if (FModuleManager::Get().IsModuleLoaded("MessageLog"))
        {
            FMessageLog Log("PIE");
            Log.Warning(LOCTEXT("LoadingOverlayClassNotSet", "LoadingOverlayClass is not set in GameMode. Fallback to no overlay."));
        }
#endif
        UE_LOG(LogFlyingGameMode, Warning, TEXT("LoadingOverlayClass is not set in GameMode. Fallback to no overlay."));

        // 最小時間制約を解除して通常進行
        bMinDurationSatisfied = true;
        ProceedUnloadPreviousStage();
        return;
    }

    if (!LoadingOverlayWidget)
    {
        LoadingOverlayWidget = CreateWidget<ULoadingOverlayBase>(World, LoadingOverlayClass);
        LoadingOverlayWidget->AddToViewport(/*ZOrder*/ LoadingOverlayZOrder);
    }

    // 映像再生とフェードイン
    LoadingOverlayWidget->SetRenderOpacity(0.f);
    LoadingOverlayWidget->PlayLoadingVideo();
    bIsOverlayVisible = true;

    // フェードインが完了するまで最後の向きで前進を続ける
    check(nullptr != CachedFlyingPlayerController);
    CachedFlyingPlayerController->ContinueMoveForwardAfterSplineEndDuringFade(FadeInDurationSeconds);
    StartFade(/*From*/ 0.f, /*To*/ 1.f, FadeInDurationSeconds, /*bIsFadeIn*/ true);

    // 最低表示時間のタイマー
    if (LoadingMinDurationSeconds > 0.f)
    {
        World->GetTimerManager().SetTimer(MinDurationTimerHandle, this, &AFlyingGameMode::OnMinDurationReached, LoadingMinDurationSeconds, /*bLoop*/ false);
    }
    else
    {
        bMinDurationSatisfied = true;
    }

    // フェードイン完了後にアンロードするため、フラグを立てる
    bPendingUnloadAfterFadeIn = true;
}

void AFlyingGameMode::StartFade(float FromOpacity, float ToOpacity, float Duration, bool /*bIsFadeIn*/)
{
    UWorld* World = GetWorld();
    check(World != nullptr);
    check(LoadingOverlayWidget != nullptr);

    FadeFromOpacity = FromOpacity;
    FadeToOpacity = ToOpacity;
    FadeTotalDuration = FMath::Max(0.01f, Duration);
    FadeElapsedSeconds = 0.f;

    World->GetTimerManager().SetTimer(FadeTimerHandle, this, &AFlyingGameMode::TickFade, FadeTickSeconds, /*bLoop*/ true);
}

void AFlyingGameMode::TickFade()
{
    UWorld* World = GetWorld();
    check(World != nullptr);
    check(LoadingOverlayWidget != nullptr);

    FadeElapsedSeconds += FadeTickSeconds;
    const float Alpha = FMath::Clamp(FadeElapsedSeconds / FadeTotalDuration, 0.f, 1.f);
    const float Opacity = FMath::Lerp(FadeFromOpacity, FadeToOpacity, Alpha);
    LoadingOverlayWidget->SetRenderOpacity(Opacity);

    if (Alpha >= 1.f)
    {
        World->GetTimerManager().ClearTimer(FadeTimerHandle);
        if (FMath::IsNearlyEqual(FadeToOpacity, 1.f))
        {
            OnFadeInFinished();
        }
        else
        {
            OnFadeOutFinished();
        }
    }
}

void AFlyingGameMode::OnFadeInFinished()
{
    // フェードイン完了。必要ならここでアンロードを開始
    if (bPendingUnloadAfterFadeIn)
    {
        bPendingUnloadAfterFadeIn = false;
        ProceedUnloadPreviousStage();
    }
}

void AFlyingGameMode::OnFadeOutFinished()
{
    check(LoadingOverlayWidget != nullptr);

    LoadingOverlayWidget->StopLoadingVideo();
    LoadingOverlayWidget->RemoveFromParent();
    LoadingOverlayWidget = nullptr;
    bIsOverlayVisible = false;

    // フェードアウト完了後に移動開始（Opening 特例は OnStageLoaded 内でハンドリング済）
    if (CurrentLoadedLevel)
    {
        // 初回ステージの場合は Opening 再生へ
        if (0 == CurrentPathIndex)
        {
            PlayOpeningSequence();
        }
        else
        {
            StartMovementForCurrentStage(CurrentLoadedLevel);
        }
    }
}

void AFlyingGameMode::OnMinDurationReached()
{
    bMinDurationSatisfied = true;
    TryFinishTransitionAfterLoadAndMin();
}

void AFlyingGameMode::TryFinishTransitionAfterLoadAndMin()
{
    // 次のステージのロード完了 かつ 最低表示時間達成 を満たしていればフェードアウト
    if (bNextStageReady && bMinDurationSatisfied)
    {
        if (LoadingOverlayWidget && bIsOverlayVisible)
        {
            // フェードアウト中に次ステージのスプライン先頭へ等速でプレ移動させる
            if (CurrentLoadedLevel && CurrentPathIndex != 0)
            {
                if (APathActor* PathActor = FindUniquePathActorInStreamingLevel(CurrentLoadedLevel))
                {
                    USplineComponent* SplineComp = PathActor->GetSplineComponent();
                    check(SplineComp != nullptr);

                    check(CachedFlyingPlayerController != nullptr);
                    CachedFlyingPlayerController->PrepareMoveTowardsSplineStartDuringFade(SplineComp, FadeOutDurationSeconds);
                }
            }

            // 新しいステージへの切替開始時（ロード画面フェードアウト開始時）にVMを更新
            UpdateViewModelStageState();
            StartFade(/*From*/ 1.f, /*To*/ 0.f, FadeOutDurationSeconds, /*bIsFadeIn*/ false);
        }
        else
        {
            // オーバーレイがない場合でも進行を継続
            if (CurrentLoadedLevel)
            {
                if (0 == CurrentPathIndex)
                {
                    // 最初のステージでも、切替開始時点で番号更新
                    UpdateViewModelStageState();
                    PlayOpeningSequence();
                }
                else
                {
                    // オーバーレイなしで切替開始する場合も、開始時にVM更新
                    UpdateViewModelStageState();
                    StartMovementForCurrentStage(CurrentLoadedLevel);
                    if (InGameInfoWidgetClass && !InGameInfoWidget)
                    {
                        ShowInGameWidget();
                    }
                }
            }
        }
    }
}

APathActor* AFlyingGameMode::FindUniquePathActorInStreamingLevel(ULevel* Level) const
{
    check(Level != nullptr);
    
    APathActor* FirstFound = nullptr;
    int32 FoundCount = 0;
    for (AActor* Actor : Level->Actors)
    {
        APathActor* PathActor = Cast<APathActor>(Actor);
        // skip if not APathActor
        if (!PathActor)
        {
            continue;
        }
        
        ++FoundCount;
        if (1 == FoundCount)
        {
            FirstFound = PathActor;
        }
        
        if (FoundCount > 1)
        {
            break;
        }
    }
    
    const FString LevelAssetPath = Level->GetPackage() ? Level->GetPackage()->GetName() : TEXT("<UnknownLevel>");
    if (FoundCount == 0)
    {
#if WITH_EDITOR
        if (FModuleManager::Get().IsModuleLoaded("MessageLog"))
        {
            FMessageLog Log("PIE");
            Log.Error(FText::Format(
                LOCTEXT("NoPathActorInLevelFmt", "No PathActor found in streamed level: {0}. Place exactly one PathActor per sublevel."),
                FText::FromString(LevelAssetPath)));
        }
#endif
        UE_LOG(LogFlyingGameMode, Error, TEXT("No PathActor found in streamed level: %s. Place exactly one PathActor per sublevel."), *LevelAssetPath);
        return nullptr;
    }

    if (FoundCount > 1)
    {
#if WITH_EDITOR
        if (FModuleManager::Get().IsModuleLoaded("MessageLog"))
        {
            FMessageLog Log("PIE");
            Log.Warning(FText::Format(
                LOCTEXT("MultiplePathActorsInLevelFmt", "Multiple PathActor actors found in streamed level: {0}. The first one will be used."),
                FText::FromString(LevelAssetPath)));
        }
#endif
        UE_LOG(LogFlyingGameMode, Warning, TEXT("Multiple PathActor actors found in streamed level: %s. The first one will be used."), *LevelAssetPath);
    }

    return FirstFound;
}

APawn* AFlyingGameMode::FindPawnByTagInPersistentLevel(const FName Tag) const
{
    UWorld* World = GetWorld();
    check(World != nullptr);

    ULevel* Persistent = World->PersistentLevel;
    check(Persistent != nullptr);

    for (TActorIterator<APawn> It(World); It; ++It)
    {
        APawn* Pawn = *It;
        if (!Pawn)
        {
            continue;
        }

        if (Pawn->GetLevel() != Persistent)
        {
            continue;
        }

        if (Pawn->ActorHasTag(Tag))
        {
            return Pawn;
        }
    }

    return nullptr;
}

#undef LOCTEXT_NAMESPACE
