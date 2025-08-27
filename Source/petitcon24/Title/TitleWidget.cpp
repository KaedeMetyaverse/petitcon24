#include "TitleWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void UTitleWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	check(StartButton);
	StartButton->OnClicked.AddDynamic(this, &UTitleWidget::HandleStartClicked);

	if (!LevelToOpen.ToSoftObjectPath().IsValid()) {
#if WITH_EDITOR
		if (FModuleManager::Get().IsModuleLoaded("MessageLog")) {
			FMessageLog Log("PIE");
			Log.Error(NSLOCTEXT("TitleWidget", "LevelToOpenNotSet", "LevelToOpen is not set in TitleWidget. Please assign a level to open on start."));
		}
#endif
		UE_LOG(LogTemp, Error,
		       TEXT("LevelToOpen is not set in TitleWidget. Please assign a level "
		            "to open on start."));
		return;
	}
}

void UTitleWidget::HandleStartClicked()
{
	const FString LongPackageName = LevelToOpen.ToSoftObjectPath().GetLongPackageName();
	if (!ensureAlways(!LongPackageName.IsEmpty()))
	{
		return;
	}

	const FName LevelName(*LongPackageName);
	UGameplayStatics::OpenLevel(this, LevelName);
}
