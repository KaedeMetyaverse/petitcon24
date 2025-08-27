#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "UObject/ObjectPtr.h"
#include "GameFramework/PlayerState.h"
class USceneComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UFloatingPawnMovement;
class UArrowComponent;
class UFlyingPawnCollisionComponent;
struct FHitResult;
#include "IHasHealth.h"
#include "FlyingPawn.generated.h"

UCLASS(abstract)
class PETITCON24_API AFlyingPawn : public APawn
{
    GENERATED_BODY()

public:
    AFlyingPawn();

    void EnableCollision();
    void DisableCollision();

    // Pawn と補助コリジョンアクタの両方へ Location を適用
    void SetLocationWithCollision(const FVector& NewLocation, bool bSweep = false);

    // HP変更の受信フック（実装は任意）
    UFUNCTION(BlueprintImplementableEvent, Category="Health")
    void ReceiveHealthChanged(int32 NewHP);

protected:
    virtual void PostInitializeComponents() override;
    virtual void OnPlayerStateChanged(APlayerState* NewPlayerState, APlayerState* OldPlayerState) override;
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    TObjectPtr<USceneComponent> Root;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    TObjectPtr<USkeletalMeshComponent> SkeletalMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    TObjectPtr<UCameraComponent> Camera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    TObjectPtr<UFloatingPawnMovement> MovementComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    TObjectPtr<UFlyingPawnCollisionComponent> CollisionComponent;

    UFUNCTION()
    void OnPawnHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);
    
    UFUNCTION()
    void EndInvincibility();

private:
    // 無敵時間（秒）
    static constexpr float InvincibilitySeconds = 3.0f;

#if WITH_EDITORONLY_DATA
	/** Component shown in the editor only to indicate character facing */
	UPROPERTY()
	TObjectPtr<UArrowComponent> ArrowComponent;
#endif
    
    FTimerHandle InvincibilityTimerHandle;

private: // HP→外観: デリゲート購読
    void BindHealthChangedDelegate(APlayerState* NewPlayerState);
    void UnbindHealthChangedDelegate(APlayerState* OldPlayerState);
    void HandleHealthChanged(int32 NewHP);

    // 補助アクタのヒットイベントへバインド
    void BindCollisionHit();

    FDelegateHandle HealthChangedHandle;
};
