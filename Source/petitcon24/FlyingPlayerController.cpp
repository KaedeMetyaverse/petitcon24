#include "FlyingPlayerController.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Pawn.h"

DEFINE_LOG_CATEGORY_STATIC(LogFlyingPlayerController, Log, All);

AFlyingPlayerController::AFlyingPlayerController()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AFlyingPlayerController::ApplyPawnTransformAtDistanceAlongSpline(const float Distance, const bool bSweep)
{
    USplineComponent* Spline = ActiveSpline;
    check(Spline != nullptr);

    APawn* ControlledPawn = GetPawn();
    if (nullptr == ControlledPawn)
    {
        UE_LOG(LogFlyingPlayerController, Error, TEXT("ControlledPawn is nullptr"));
        UE_DEBUG_BREAK();
        return;
    }

    const FVector NewLocation = Spline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
    const FRotator NewRotation = Spline->GetRotationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
    ControlledPawn->SetActorLocationAndRotation(NewLocation, NewRotation, /*bSweep*/ bSweep);
}

void AFlyingPlayerController::StartMoveAlongSpline(TObjectPtr<USplineComponent> InSpline)
{
    check(InSpline != nullptr);
    ActiveSpline = InSpline;
    CurrentSplineDistance = 0.f;
    bIsFollowingSpline = true;

    // スタート地点へ即時移動（衝突無しでテレポート相当）
    ApplyPawnTransformAtDistanceAlongSpline(/*Distance*/ 0.f, /*bSweep*/ false);
}

void AFlyingPlayerController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!bIsFollowingSpline)
    {
        return;
    }

    USplineComponent* Spline = ActiveSpline;
    check(Spline != nullptr);

    // 進行距離を更新
    CurrentSplineDistance += SplineMoveSpeed * DeltaSeconds;

    const float SplineLength = Spline->GetSplineLength();
    if (CurrentSplineDistance >= SplineLength)
    {
        CurrentSplineDistance = SplineLength;
        bIsFollowingSpline = false;
    }

    ApplyPawnTransformAtDistanceAlongSpline(/*Distance*/ CurrentSplineDistance, /*bSweep*/ true);

    // 完了したフレームで通知（最後の位置へ反映後）
    if (!bIsFollowingSpline)
    {
        MoveAlongSplineFinishedDelegate.Broadcast();
    }
}
