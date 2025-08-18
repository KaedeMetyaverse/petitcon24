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
    StartSequence();
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

    ULevel* LoadedLevel = Streaming->GetLoadedLevel();
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

#undef LOCTEXT_NAMESPACE
