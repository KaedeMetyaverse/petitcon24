#include "TitleWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Internationalization/Text.h"
#if WITH_EDITOR
#include "Logging/MessageLog.h"
#include "Modules/ModuleManager.h"
#endif

void UTitleWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (StartButton)
	{
		StartButton->OnClicked.AddDynamic(this, &UTitleWidget::HandleStartClicked);
	}
}

void UTitleWidget::HandleStartClicked()
{
	const FString LongPackageName = LevelToOpen.ToSoftObjectPath().GetLongPackageName();
	if (!LongPackageName.IsEmpty())
	{
		const FName LevelName(*LongPackageName);
		UGameplayStatics::OpenLevel(this, LevelName);
	}
	else
	{
#if WITH_EDITOR
		if (FModuleManager::Get().IsModuleLoaded("MessageLog"))
		{
			FMessageLog Log("PIE");
			Log.Warning(NSLOCTEXT("TitleWidget", "LevelToOpenNotSet", "LevelToOpen is not set on TitleWidget. Please assign a World asset."));
		}
#endif
		UE_LOG(LogTemp, Warning, TEXT("LevelToOpen is not set on TitleWidget. Please assign a World asset."));
	}
}


