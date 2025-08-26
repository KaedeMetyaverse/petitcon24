#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "IHasHealth.h"
#include "FlyingPlayerState.generated.h"

UCLASS()
class PETITCON24_API AFlyingPlayerState : public APlayerState, public IHasHealth
{
    GENERATED_BODY()

public:
    AFlyingPlayerState();

    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // IHasHealth 実装
    virtual int32 GetCurrentHP() const override;
    virtual bool IsDead() const override;
    virtual void ApplyDamage(float DamageAmount) override;
    virtual FOnHealthChangedDelegate& OnHealthChanged() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Health", meta=(ClampMin="0", UIMin="0"))
    int32 InitialHP = 2;

private:
    UPROPERTY(ReplicatedUsing=OnRep_CurrentHP, VisibleInstanceOnly, BlueprintReadOnly, Category="Health", meta=(AllowPrivateAccess="true"))
    int32 CurrentHP = 0;

    UFUNCTION()
    void OnRep_CurrentHP();

    FOnHealthChangedDelegate HealthChangedDelegate;
};
