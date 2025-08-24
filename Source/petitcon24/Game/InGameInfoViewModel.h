#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "InGameInfoViewModel.generated.h"

UCLASS(BlueprintType)
class PETITCON24_API UInGameInfoViewModel : public UMVVMViewModelBase
{
    GENERATED_BODY()

public:
    // 総ステージ数
    UPROPERTY(BlueprintReadOnly, FieldNotify, Category="InGameInfo")
    int32 StageCount = 0;

    // 現在のステージ番号（1始まりで扱う運用を想定。UI側でそのまま表示可能）
    UPROPERTY(BlueprintReadOnly, FieldNotify, Category="InGameInfo")
    int32 CurrentStageNumber = 0;

    // 全ステージのパス総距離
    UPROPERTY(BlueprintReadOnly, FieldNotify, Category="InGameInfo")
    double TotalPathLength = 0.0;

    // 現在までの進行済み距離（パス長）
    UPROPERTY(BlueprintReadOnly, FieldNotify, Category="InGameInfo")
    double TraveledPathLength = 0.0;

public:
    // C++専用セッター（BPからは取得のみ）
    void SetStageCount(int32 InStageCount);

    void SetCurrentStageNumber(int32 InCurrentNumber);

    void SetTotalPathLength(double InTotal);

    void SetTraveledPathLength(double InTraveled);
};
