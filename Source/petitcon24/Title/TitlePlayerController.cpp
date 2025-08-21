#include "TitlePlayerController.h"
#include "TitleWidget.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "TitleGameMode.h"
#include "Internationalization/Text.h"
#if WITH_EDITOR
#include "Logging/MessageLog.h"
#include "Modules/ModuleManager.h"
#endif

DEFINE_LOG_CATEGORY_STATIC(LogTitlePlayerController, Log, All);
#define LOCTEXT_NAMESPACE "TitlePlayerController"

namespace
{
    constexpr int32 TitleWidgetZOrder = 100;
}

ATitlePlayerController::ATitlePlayerController()
{
}

void ATitlePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController())
	{
		return;
	}

	// GameMode からタイトルのウィジェットクラスを取得
	TSubclassOf<UTitleWidget> WidgetClass = nullptr;

    AGameModeBase* GM = UGameplayStatics::GetGameMode(this);
    check(GM != nullptr);

    ATitleGameMode* TitleGM = Cast<ATitleGameMode>(GM);
    if (!TitleGM) {
#if WITH_EDITOR
        if (FModuleManager::Get().IsModuleLoaded("MessageLog"))
        {
            FMessageLog Log("PIE");
            Log.Error(LOCTEXT("TitleGMIsNull", "GameMode is not ATitleGameMode. Please set TitleGameMode in World Settings."));
        }
#endif
        UE_LOG(LogTitlePlayerController, Error, TEXT("GameMode is not ATitleGameMode. Please set TitleGameMode in World Settings."));
        return;
    }

    WidgetClass = TitleGM->TitleWidgetClass;
    if (!WidgetClass) {
#if WITH_EDITOR
        if (FModuleManager::Get().IsModuleLoaded("MessageLog"))
        {
            FMessageLog Log("PIE");
            Log.Error(LOCTEXT("TitleWidgetClassNull", "TitleWidgetClass is not set on TitleGameMode. Please assign a Widget Blueprint."));
        }
#endif
        UE_LOG(LogTitlePlayerController, Error, TEXT("TitleWidgetClass is not set on TitleGameMode. Please assign a Widget Blueprint."));
        return;
    }

    UTitleWidget* Widget = CreateWidget<UTitleWidget>(this, WidgetClass);
    if (!Widget)
    {
        UE_LOG(LogTitlePlayerController, Error, TEXT("Failed to create TitleWidget instance."));
        return;
    }
    SpawnedTitleWidget = Widget;
    Widget->AddToViewport(/*ZOrder*/ TitleWidgetZOrder);

    // UI 操作用の入力モード
    FInputModeUIOnly InputMode;
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    InputMode.SetWidgetToFocus(Widget->TakeWidget());
    SetInputMode(InputMode);
    bShowMouseCursor = true;
}

#undef LOCTEXT_NAMESPACE
