#include "FlyingPlayerState.h"
#include "Net/UnrealNetwork.h"

AFlyingPlayerState::AFlyingPlayerState()
{
    bReplicates = true;
}

void AFlyingPlayerState::BeginPlay()
{
    Super::BeginPlay();
    CurrentHP = InitialHP;
    HealthChangedDelegate.Broadcast(CurrentHP);
}

void AFlyingPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AFlyingPlayerState, CurrentHP);
}

void AFlyingPlayerState::ApplyDamage(const float DamageAmount)
{
    if (CurrentHP > 0)
    {
        CurrentHP = FMath::Max(0, CurrentHP - static_cast<int32>(DamageAmount));
        HealthChangedDelegate.Broadcast(CurrentHP);
    }
}

void AFlyingPlayerState::OnRep_CurrentHP()
{
    HealthChangedDelegate.Broadcast(CurrentHP);
}

int32 AFlyingPlayerState::GetCurrentHP() const
{
    return CurrentHP;
}

bool AFlyingPlayerState::IsDead() const
{
    return CurrentHP <= 0;
}

FOnHealthChangedDelegate& AFlyingPlayerState::OnHealthChanged()
{
    return HealthChangedDelegate;
}
