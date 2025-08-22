#include "InGameInfoViewModel.h"

void UInGameInfoViewModel::SetStageCount(const int32 InStageCount)
{
    if (StageCount != InStageCount)
    {
        StageCount = InStageCount;
        UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(StageCount);
    }
}

void UInGameInfoViewModel::SetCurrentStageNumber(const int32 InCurrentNumber)
{
    if (CurrentStageNumber != InCurrentNumber)
    {
        CurrentStageNumber = InCurrentNumber;
        UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(CurrentStageNumber);
    }
}

void UInGameInfoViewModel::SetTotalSplineLength(const double InTotal)
{
    if (TotalSplineLength != InTotal)
    {
        TotalSplineLength = InTotal;
        UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(TotalSplineLength);
    }
}

void UInGameInfoViewModel::SetTraveledSplineLength(const double InTraveled)
{
    if (TraveledSplineLength != InTraveled)
    {
        TraveledSplineLength = InTraveled;
        UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(TraveledSplineLength);
    }
}