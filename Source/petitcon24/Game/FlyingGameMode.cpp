#include "FlyingGameMode.h"
#include "FlyingPlayerController.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Actor.h"
#include "EngineUtils.h"
#include "Logging/LogMacros.h"
#include "FlyingPawn.h"
#include "Modules/ModuleManager.h"
#include "Internationalization/Text.h"
#include "Engine/LevelStreaming.h"
#include "Engine/Level.h"
#include "Kismet/GameplayStatics.h"
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

    // 既存のステージ進行フローを開始
    StartSequence();
}

void AFlyingGameMode::PlayOpeningSequence()
{
    // 未設定なら即移動開始
    if (!OpeningSequence.IsValid() && !OpeningSequence.ToSoftObjectPath().IsValid())
    {
#if WITH_EDITOR
        if (FModuleManager::Get().IsModuleLoaded("MessageLog"))
        {
            FMessageLog Log("PIE");
            Log.Warning(LOCTEXT("OpeningSequenceNotSet", "OpeningSequence is not set in GameMode."));
        }
#endif
        UE_LOG(LogFlyingGameMode, Warning, TEXT("OpeningSequence is not set in GameMode."));
        StartMovementForCurrentStage(CurrentLoadedLevel);
        return;
    }

    ULevelSequence* SequenceAsset = OpeningSequence.LoadSynchronous();
    check(SequenceAsset != nullptr);

    ALevelSequenceActor* OutActor = nullptr;
    ULevelSequencePlayer* Player = ULevelSequencePlayer::CreateLevelSequencePlayer(this, SequenceAsset, FMovieSceneSequencePlaybackSettings(), OutActor);
    check(Player != nullptr);
    check(OutActor != nullptr);

    OpeningSequencePlayer = Player;
    OpeningSequenceActor = OutActor;

    // 再生完了イベントにバインド
    Player->OnFinished.AddDynamic(this, &AFlyingGameMode::HandlePlayOpeningSequenceFinished);

    Player->Play();
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

void AFlyingGameMode::TryStartNextPath()
{
    check(nullptr != CachedFlyingPlayerController);

    ++CurrentPathIndex;
    if (!Stages.IsValidIndex(CurrentPathIndex))
    {
        // 全行程完了
        return;
    }

    ProceedUnloadPreviousStage();
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
    LatentInfo.ExecutionFunction = FName("OnStageUnloaded");
    LatentInfo.UUID = 1001;
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
    LatentInfo.ExecutionFunction = FName("OnStageLoaded");
    LatentInfo.UUID = 1002;
    LatentInfo.Linkage = 0;
    UGameplayStatics::LoadStreamLevelBySoftObjectPtr(this, Stages[CurrentPathIndex], true /* visible */, false /* non-blocking */, LatentInfo);
}

void AFlyingGameMode::OnStageLoaded()
{
    const FString CurrLongPackageName = Stages[CurrentPathIndex].ToSoftObjectPath().GetLongPackageName();
    check(!CurrLongPackageName.IsEmpty());

    ULevelStreaming* Streaming = UGameplayStatics::GetStreamingLevel(this, FName(*CurrLongPackageName));
    if (nullptr == Streaming || !Streaming->IsLevelLoaded())
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

    // 最初のステージで Opening を再生
    if (0 == CurrentPathIndex)
    {
        PlayOpeningSequence();
        return;
    }

    // Opening不要 or 再生済みなら、移動開始
    StartMovementForCurrentStage(CurrentLoadedLevel);
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

APathActor* AFlyingGameMode::FindUniquePathActorInStreamingLevel(ULevel* Level) const
{
    check(Level != nullptr);
    
    APathActor* FirstFound = nullptr;
    int32 FoundCount = 0;
    for (AActor* Actor : Level->Actors)
    {
        APathActor* PathActor = Cast<APathActor>(Actor);
        // skip if not APathActor
        if (nullptr == PathActor)
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

APawn* AFlyingGameMode::FindPawnByTagInPersistentLevel(FName Tag) const
{
    UWorld* World = GetWorld();
    check(World != nullptr);

    ULevel* Persistent = World->PersistentLevel;
    check(Persistent != nullptr);

    for (AActor* Actor : Persistent->Actors)
    {
	    if (nullptr == Actor)
        {
            continue;
	    }

        if (Actor->ActorHasTag(Tag))
        {
            APawn* Pawn = Cast<APawn>(Actor);
            if (nullptr == Pawn) {
                UE_LOG(LogFlyingGameMode, Log, TEXT("Actor: %s has tag: %s, but is not a Pawn"), *Actor->GetName(), *Tag.ToString());
                continue;
            }

            return Pawn;
        }
    }

    return nullptr;
}

#undef LOCTEXT_NAMESPACE
