#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
class USplineComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
#include "FlyingPlayerController.generated.h"

// C++専用: スプライン移動完了を通知するデリゲート
DECLARE_MULTICAST_DELEGATE(FMoveAlongSplineFinishedDelegate);

UCLASS(abstract)
class PETITCON24_API AFlyingPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AFlyingPlayerController();

    // 指定したスプラインに沿った自動移動を開始する（C++専用）
    void StartMoveAlongSpline(TObjectPtr<USplineComponent> InSpline);

    // フェードアウト中に最初のスプラインポイントへ向けて等速移動を始める
    // FadeDurationSeconds の間だけ移動し、終了時にちょうど距離0のポイントへ到達させる
    void PrepareMoveTowardsSplineStartDuringFade(TObjectPtr<USplineComponent> InSpline, float FadeDurationSeconds);

    // スプライン終点に到達した直後、フェードインが完了するまで最後の向きに向かって等速で直進する
    void ContinueMoveForwardAfterSplineEndDuringFade(float FadeDurationSeconds);

    virtual void Tick(float DeltaSeconds) override;

    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // C++専用: スプライン移動完了デリゲートへのアクセサ
    FMoveAlongSplineFinishedDelegate& OnMoveAlongSplineFinished() { return MoveAlongSplineFinishedDelegate; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input", meta=(AllowPrivateAccess="true"))
    TArray<TObjectPtr<UInputMappingContext>> DefaultMappingContexts;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UInputAction> MoveAction;

    virtual void SetupInputComponent() override;

private:
    // 追従スピード（クラス定数）
    static constexpr float SplineMoveSpeed = 2000.f;

    // ピボットからの許容移動半径
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SplineFollow", meta=(AllowPrivateAccess="true"))
    float AllowedMoveRadius = 100.f;

private:
    TObjectPtr<USplineComponent> ActiveSpline;
    float CurrentSplineDistance = 0.f;
    bool bIsFollowingSpline = false;

    // フェード中のプレ移動
    bool bIsPreMovingTowardsSplineStart = false;
    FVector PreMoveTargetLocation = FVector::ZeroVector;
    FRotator PreMoveTargetRotation = FRotator::ZeroRotator;
    float PreMoveRemainingSeconds = 0.f;
    float PreMoveTotalSeconds = 0.f;

    // フェードイン中のポスト移動（終点から前進）
    bool bIsPostMovingForwardAfterSplineEnd = false;
    FVector PostMoveForwardDirection = FVector::ForwardVector;
    float PostMoveRemainingSeconds = 0.f;
    float PostMoveTotalSeconds = 0.f;

    // スプライン上の現在のピボット（ApplyPawnTransformAtDistanceAlongSpline で更新）
    FVector CurrentSplinePivotLocation = FVector::ZeroVector;

    // UpdatedComponent の初期ローカル位置（Pawn のルートに対する相対位置）
    FVector UpdatedComponentInitialLocalOffset = FVector::ZeroVector;

    // C++専用: 完了通知用デリゲート
    FMoveAlongSplineFinishedDelegate MoveAlongSplineFinishedDelegate;

    // 指定距離のスプライン位置・回転をPawnへ適用
    void ApplyPawnTransformAtDistanceAlongSpline(float Distance, bool bSweep);

    // 入力: 移動
    void HandleMoveInput(const struct FInputActionValue& Value);

    // 入力処理の中身（Pawn に AddMovementInput する）
    void DoMoveControlledPawn(float Right, float Up);

    // 現在所持している Pawn の UpdatedComponent を取得（null 安全）
    class USceneComponent* GetPawnUpdatedComponent() const;
};
