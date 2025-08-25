#include "HealthComponent.h"

UHealthComponent::UHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent         = true;
}

void UHealthComponent::InitializeComponent()
{
    Super::InitializeComponent();
    CurrentHP = InitialHP;
    HealthChangedDelegate.Broadcast(CurrentHP);
}

void UHealthComponent::ApplyDamage(const float DamageAmount)
{
    if (CurrentHP > 0)
    {
        CurrentHP = FMath::Max(0, CurrentHP - DamageAmount);
        HealthChangedDelegate.Broadcast(CurrentHP);
    }
}
