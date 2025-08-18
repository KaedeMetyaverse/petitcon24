#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TitleGameMode.generated.h"

class UTitleWidget;

UCLASS()
class PETITCON24_API ATitleGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATitleGameMode();

public:
	// タイトル画面で使用するウィジェットのクラス（BPで設定推奨）
	UPROPERTY(EditDefaultsOnly, Category="Title")
	TSubclassOf<UTitleWidget> TitleWidgetClass;
};
