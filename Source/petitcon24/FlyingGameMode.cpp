#include "FlyingGameMode.h"
#include "FlyingPlayerController.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Actor.h"
#include "EngineUtils.h"
#include "Logging/LogMacros.h"
#include "Modules/ModuleManager.h"
#include "Internationalization/Text.h"
#if WITH_EDITOR
#include "Logging/MessageLog.h"
#endif

DEFINE_LOG_CATEGORY_STATIC(LogFlyingGameMode, Log, All);

#define LOCTEXT_NAMESPACE "FlyingGameMode"
AFlyingGameMode::AFlyingGameMode()
{
    // Set FlyingPlayerController as the default player controller
    PlayerControllerClass = AFlyingPlayerController::StaticClass();
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

    // レベル上の PathSequenceInfo を探索し、最初に見つかったものから配列を取得
    int32 FoundCount = 0;
    TObjectPtr<APathSequenceInfo> FirstFound = nullptr;
    for (TActorIterator<APathSequenceInfo> It(World); It; ++It)
    {
        TObjectPtr<APathSequenceInfo> Info = *It;
        check(Info != nullptr);
        ++FoundCount;

        if (nullptr == FirstFound)
        {
            FirstFound = Info;
        }

        if (FoundCount > 1)
        {
            break;
        }
    }

    if (FoundCount == 0)
    {
        // MessageLog と OutputLog にエラー
#if WITH_EDITOR
        if (FModuleManager::Get().IsModuleLoaded("MessageLog"))
        {
            FMessageLog Log("PIE");
            Log.Error(LOCTEXT("NoPathSequenceInfo", "No PathSequenceInfo found in level. Place exactly one PathSequenceInfo."));
        }
#endif
        UE_LOG(LogFlyingGameMode, Error, TEXT("No PathSequenceInfo found in level. Place exactly one PathSequenceInfo."));
        return;
    }

    if (FoundCount > 1)
    {
        
#if WITH_EDITOR
        if (FModuleManager::Get().IsModuleLoaded("MessageLog"))
        {
            FMessageLog Log("PIE");
            Log.Warning(LOCTEXT("MultiplePathSequenceInfo", "Multiple PathSequenceInfo actors found. The first one will be used."));
        }
#endif
        UE_LOG(LogFlyingGameMode, Warning, TEXT("Multiple PathSequenceInfo actors found. The first one will be used."));
    }

    check(FirstFound != nullptr);
    PathActorsSequence = FirstFound->PathActorsSequence;

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
    if (!PathActorsSequence.IsValidIndex(CurrentPathIndex))
    {
        // 全行程完了
        return;
    }

    TObjectPtr<APathActor> PathActor = PathActorsSequence[CurrentPathIndex];
    check(PathActor != nullptr);

    // PathActorからスプラインコンポーネントを取得
    TObjectPtr<USplineComponent> SplineComp = PathActor->GetSplineComponent();
    check(SplineComp != nullptr);

    CachedFlyingPlayerController->StartMoveAlongSpline(SplineComp);
}

void AFlyingGameMode::HandleMoveAlongSplineFinished()
{
    // 次の PathActor のスプラインへ
    TryStartNextPath();
}

#undef LOCTEXT_NAMESPACE
