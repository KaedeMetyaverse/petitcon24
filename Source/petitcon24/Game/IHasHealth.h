#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HealthTypes.h"
#include "IHasHealth.generated.h"

UINTERFACE(BlueprintType)
class PETITCON24_API UHasHealth : public UInterface
{
    GENERATED_BODY()
};

class PETITCON24_API IHasHealth
{
    GENERATED_BODY()

public:
    // 現在HPの取得
    virtual int32 GetCurrentHP() const = 0;

    // 現在HPが0以下か
    virtual bool IsDead() const = 0;

    // ダメージ適用
    virtual void ApplyDamage(float DamageAmount) = 0;

    // 体力変更時の通知デリゲート（参照返し）
    virtual FOnHealthChangedDelegate& OnHealthChanged() = 0;
};
