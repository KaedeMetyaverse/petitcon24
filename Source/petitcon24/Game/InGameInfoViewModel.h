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

    // 全ステージのスプライン総距離（未実装: 値は今はセットしない）
    UPROPERTY(BlueprintReadOnly, FieldNotify, Category="InGameInfo")
    double TotalSplineLength = 0.0;

    // 現在までの進行済み距離（未実装: 値は今はセットしない）
    UPROPERTY(BlueprintReadOnly, FieldNotify, Category="InGameInfo")
    double TraveledSplineLength = 0.0;

public:
    // C++専用セッター（BPからは取得のみ）
    void SetStageCount(const int32 InStageCount)
    {
        if (StageCount != InStageCount)
        {
            StageCount = InStageCount;
            UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(StageCount);
        }
    }

    void SetCurrentStageNumber(const int32 InCurrentNumber)
    {
        if (CurrentStageNumber != InCurrentNumber)
        {
            CurrentStageNumber = InCurrentNumber;
            UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(CurrentStageNumber);
        }
    }

    void SetTotalSplineLength(const double InTotal)
    {
        if (TotalSplineLength != InTotal)
        {
            TotalSplineLength = InTotal;
            UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(TotalSplineLength);
        }
    }

    void SetTraveledSplineLength(const double InTraveled)
    {
        if (TraveledSplineLength != InTraveled)
        {
            TraveledSplineLength = InTraveled;
            UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(TraveledSplineLength);
        }
    }
};
