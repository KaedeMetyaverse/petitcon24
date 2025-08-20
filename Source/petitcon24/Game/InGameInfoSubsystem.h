#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "InGameInfoViewModel.h"
#include "InGameInfoSubsystem.generated.h"

UCLASS()
class PETITCON24_API UInGameInfoSubsystem : public ULocalPlayerSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintPure, Category = "InGameUI")
    UInGameInfoViewModel* GetInGameInfoViewModel() const { return ViewModel; }

    // C++ からの更新ヘルパ（必要に応じて使用）
    void SetStageCount(const int32 InStageCount);
    void SetCurrentStageNumber(const int32 InCurrentNumber);

private:
    UPROPERTY()
    TObjectPtr<UInGameInfoViewModel> ViewModel;
};
