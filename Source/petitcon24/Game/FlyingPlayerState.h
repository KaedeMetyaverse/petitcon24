#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "FlyingPlayerState.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnHealthChanged, int32 /*NewHP*/);

UCLASS()
class PETITCON24_API AFlyingPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    AFlyingPlayerState();

    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    int32 GetCurrentHP() const;
    bool IsDead() const;

    void ApplyDamage(float DamageAmount);

    FOnHealthChanged& OnHealthChanged();

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Health", meta=(ClampMin="0", UIMin="0"))
    int32 InitialHP = 2;

private:
    UPROPERTY(ReplicatedUsing=OnRep_CurrentHP, VisibleInstanceOnly, BlueprintReadOnly, Category="Health", meta=(AllowPrivateAccess="true"))
    int32 CurrentHP = 0;

    UFUNCTION()
    void OnRep_CurrentHP();

    FOnHealthChanged HealthChangedDelegate;
};
