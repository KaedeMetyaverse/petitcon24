#include "TitleWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

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
}


