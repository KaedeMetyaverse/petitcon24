#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoadingOverlayBase.generated.h"

UCLASS(Abstract, Blueprintable)
class PETITCON24_API ULoadingOverlayBase : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintImplementableEvent, Category="LoadingOverlay")
    void PlayLoadingVideo();

    UFUNCTION(BlueprintImplementableEvent, Category="LoadingOverlay")
    void StopLoadingVideo();
};


