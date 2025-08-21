#include "InGameInfoSubsystem.h"
#include "InGameInfoViewModel.h"

void UInGameInfoSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    ViewModel = NewObject<UInGameInfoViewModel>(this);
}

void UInGameInfoSubsystem::Deinitialize()
{
    ViewModel = nullptr;
    Super::Deinitialize();
}

void UInGameInfoSubsystem::SetStageCount(const int32 InStageCount)
{
    ViewModel->SetStageCount(InStageCount);
}

void UInGameInfoSubsystem::SetCurrentStageNumber(const int32 InCurrentNumber)
{
    ViewModel->SetCurrentStageNumber(InCurrentNumber);
}
