#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnHealthChanged, int32 /*NewHP*/);

/**
 * 単純なHP管理用コンポーネント。
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PETITCON24_API UHealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHealthComponent();
    virtual void InitializeComponent() override;

    /** 現在のHPを取得 */
    int32 GetCurrentHP() const { return CurrentHP; }

    /** 死亡判定（HP<=0） */
    bool IsDead() const { return CurrentHP <= 0; }

    /** ダメージ適用（常に1減少） */
    void ApplyDamage(float DamageAmount);

    /** HP 変更を C++ へ通知（現在HPが引数） */
    FOnHealthChanged& OnHealthChanged() { return HealthChangedDelegate; }

public:
    /** 初期HP（ゲーム開始時の値）。既定値 2 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Health", meta=(ClampMin="0", UIMin="0"))
    int32 InitialHP = 2;

private:
    /** 現在HP */
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Health", meta=(AllowPrivateAccess="true"))
    int32 CurrentHP = 0;

    /** 内部: HP 変更デリゲート */
    FOnHealthChanged HealthChangedDelegate;
};
