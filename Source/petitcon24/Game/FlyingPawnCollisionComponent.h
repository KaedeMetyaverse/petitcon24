#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "FlyingPawnCollisionComponent.generated.h"

class AFlyingPawnCollisionActor;

/**
 * FlyingPawnCollisionComponent
 * - BeginPlay で不可視の CollisionComponent を持つ補助アクタをスポーン
 * - このコンポーネント自身にアタッチ
 * - EndPlay で補助アクタを破棄
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PETITCON24_API UFlyingPawnCollisionComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    UFlyingPawnCollisionComponent();
    
    // 衝突用の補助アクタ取得（null の可能性あり）
    UFUNCTION(BlueprintCallable, Category="Collision")
    class AFlyingPawnCollisionActor* GetSpawnedActor() const { return SpawnedCollisionActor; }

    // 衝突プロファイル名を切り替える（補助アクタの Sphere へ適用）
    UFUNCTION(BlueprintCallable, Category="Collision")
    void SetSpawnedCollisionProfile(FName InProfileName) const;

protected:
    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // 生成する Sphere の半径
    UPROPERTY(EditAnywhere, Category="CollisionSetup")
    float SphereRadius;

private:
    // Spawn した補助アクタ
    UPROPERTY(Transient)
    TObjectPtr<AFlyingPawnCollisionActor> SpawnedCollisionActor;
};
