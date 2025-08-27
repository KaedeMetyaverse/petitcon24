#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "petitcon24GameSubsystem.generated.h"

UCLASS()
class PETITCON24_API UPetitcon24GameSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="State")
    bool GetHasDiedOnce() const { return bHasDiedOnce; }

    UFUNCTION(BlueprintCallable, Category="State")
    void SetHasDiedOnce(const bool bValue) { bHasDiedOnce = bValue; }

private:
    UPROPERTY()
    bool bHasDiedOnce = false;
};


