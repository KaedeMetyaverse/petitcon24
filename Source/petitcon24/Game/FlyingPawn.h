#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "UObject/ObjectPtr.h"
class USphereComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UFloatingPawnMovement;
class UArrowComponent;
struct FHitResult;
#include "FlyingPawn.generated.h"

UCLASS(abstract)
class PETITCON24_API AFlyingPawn : public APawn
{
    GENERATED_BODY()

public:
    AFlyingPawn();

protected:
    virtual void BeginPlay() override;
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    TObjectPtr<USphereComponent> RootSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    TObjectPtr<USkeletalMeshComponent> SkeletalMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    TObjectPtr<UCameraComponent> Camera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    TObjectPtr<UFloatingPawnMovement> MovementComponent;

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
};


