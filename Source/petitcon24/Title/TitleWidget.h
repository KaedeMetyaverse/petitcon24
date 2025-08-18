#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UObject/SoftObjectPtr.h"
#include "Components/Button.h"
#include "TitleWidget.generated.h"

UCLASS()
class PETITCON24_API UTitleWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// エディターで指定可能: スタート時に遷移するレベル（World）
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Title")
	TSoftObjectPtr<UWorld> LevelToOpen;

protected:
	// BP で配置した "StartButton" を自動バインド
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> StartButton;

	virtual void NativeOnInitialized() override;

private:
	UFUNCTION()
	void HandleStartClicked();
};
