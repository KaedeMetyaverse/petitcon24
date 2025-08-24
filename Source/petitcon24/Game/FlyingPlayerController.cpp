#include "FlyingPlayerController.h"
#include "Components/SplineComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PawnMovementComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputActionValue.h"
#if WITH_EDITOR
#include "Logging/MessageLog.h"
#include "Modules/ModuleManager.h"
#endif

DEFINE_LOG_CATEGORY_STATIC(LogFlyingPlayerController, Log, All);

#define LOCTEXT_NAMESPACE "FlyingPlayerController"

AFlyingPlayerController::AFlyingPlayerController()
{
    PrimaryActorTick.bCanEverTick = true;
}

float AFlyingPlayerController::ComputeMovementStep(const float DeltaSeconds) const
{
    return SplineMoveSpeed * DeltaSeconds;
}

USceneComponent* AFlyingPlayerController::GetPawnUpdatedComponent() const
{
    if (const APawn* ControlledPawn = GetPawn())
    {
        if (const UPawnMovementComponent* MoveComp = ControlledPawn->GetMovementComponent())
        {
            return MoveComp->UpdatedComponent;
        }
    }
    return nullptr;
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
    // 位置は Pawn へ、回転はコントローラーへ適用（Pawn は bUseControllerRotation* で追従）
    ControlledPawn->SetActorLocation(NewLocation, /*bSweep*/ bSweep);
    SetControlRotation(NewRotation);

    // ピボットを更新
    CurrentSplinePivotLocation = NewLocation;
}

void AFlyingPlayerController::StartMoveAlongSpline(TObjectPtr<USplineComponent> InSpline)
{
    check(InSpline != nullptr);
    ActiveSpline = InSpline;
    CurrentSplineDistance = 0.f;
    // プレ移動フラグは終了
    bIsPreMovingTowardsSplineStart = false;
    // ポスト移動も念のため終了
    bIsPostMovingForwardAfterSplineEnd = false;
    bIsFollowingSpline = true;

    // スタート地点へ即時移動（衝突無しでテレポート相当）
    ApplyPawnTransformAtDistanceAlongSpline(/*Distance*/ 0.f, /*bSweep*/ false);
}

void AFlyingPlayerController::PrepareMoveTowardsSplineStartDuringFade(TObjectPtr<USplineComponent> InSpline, float FadeDurationSeconds)
{
    check(InSpline != nullptr);

    // ステート初期化
    ActiveSpline = InSpline;
    bIsFollowingSpline = false; // 本番追従はまだ開始しない
    bIsPreMovingTowardsSplineStart = true;

    // 目標（スプライン距離0）
    const FVector TargetLocation = InSpline->GetLocationAtDistanceAlongSpline(0.f, ESplineCoordinateSpace::World);
    const FRotator TargetRotation = InSpline->GetRotationAtDistanceAlongSpline(0.f, ESplineCoordinateSpace::World);
    PreMoveTargetLocation = TargetLocation;
    PreMoveTargetRotation = TargetRotation;

    // スプライン最初の角度から見た前方方向に対して Fade 分だけ手前へ戻った位置へワープ
    // SplineMoveSpeed と FadeDurationSeconds で進める距離
    const float MoveDistance = FMath::Max(0.f, SplineMoveSpeed * FMath::Max(0.f, FadeDurationSeconds));

    APawn* ControlledPawn = GetPawn();
    check(ControlledPawn != nullptr);

    const FVector TargetForward = TargetRotation.Vector();
    const FVector StartLocation = TargetLocation - TargetForward * MoveDistance;

    // 角度はターゲットの回転を基準に
    ControlledPawn->SetActorLocation(StartLocation, /*bSweep*/ false);
    SetControlRotation(TargetRotation);
    CurrentSplinePivotLocation = StartLocation;

    // フェード残時間
    const float ClampedFade = FMath::Max(0.f, FadeDurationSeconds);
    PreMoveTotalSeconds = ClampedFade;
    PreMoveRemainingSeconds = ClampedFade;
}

void AFlyingPlayerController::ContinueMoveForwardAfterSplineEndDuringFade(float FadeDurationSeconds)
{
    // スプライン終端での向きで前進し続ける
    check(ActiveSpline != nullptr);

    const float SplineLength = ActiveSpline->GetSplineLength();
    const FRotator EndRotation = ActiveSpline->GetRotationAtDistanceAlongSpline(SplineLength, ESplineCoordinateSpace::World);
    PostMoveForwardDirection = EndRotation.Vector();

    // 残時間
    const float ClampedFade = FMath::Max(0.f, FadeDurationSeconds);
    PostMoveTotalSeconds = ClampedFade;
    PostMoveRemainingSeconds = ClampedFade;
    bIsPostMovingForwardAfterSplineEnd = (ClampedFade > 0.f);
}

void AFlyingPlayerController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    // フェード中のプレ移動（最初のスプラインポイントへ向けて等速で直進）
    if (bIsPreMovingTowardsSplineStart)
    {
        if (APawn* ControlledPawn = GetPawn())
        {
            const float Step = ComputeMovementStep(DeltaSeconds);
            FVector CurrentLocation = ControlledPawn->GetActorLocation();
            FVector ToTarget = PreMoveTargetLocation - CurrentLocation;

            const float DistanceToTarget = ToTarget.Length();
            if (DistanceToTarget <= Step || PreMoveRemainingSeconds - DeltaSeconds <= 0.f)
            {
                // 目標に到達 or 残時間終了
                ControlledPawn->SetActorLocation(PreMoveTargetLocation, /*bSweep*/ false);
                SetControlRotation(PreMoveTargetRotation);
                CurrentSplinePivotLocation = PreMoveTargetLocation;
                bIsPreMovingTowardsSplineStart = false;
            }
            else
            {
                const FVector Dir = ToTarget.GetSafeNormal(UE_SMALL_NUMBER);
                const FVector NewLocation = CurrentLocation + Dir * Step;
                ControlledPawn->SetActorLocation(NewLocation, /*bSweep*/ false);
                SetControlRotation(PreMoveTargetRotation);
                CurrentSplinePivotLocation = NewLocation;
                PreMoveRemainingSeconds -= DeltaSeconds;
            }
        }
    }

    // フェードイン中のポスト移動（終点から前方へ等速直進）
    if (bIsPostMovingForwardAfterSplineEnd)
    {
        if (APawn* ControlledPawn = GetPawn())
        {
            const float Step = ComputeMovementStep(DeltaSeconds);
            const FVector CurrentLocation = ControlledPawn->GetActorLocation();
            const FVector NewLocation = CurrentLocation + PostMoveForwardDirection * Step;
            ControlledPawn->SetActorLocation(NewLocation, /*bSweep*/ false);
            SetControlRotation(PostMoveForwardDirection.Rotation());
            CurrentSplinePivotLocation = NewLocation;

            PostMoveRemainingSeconds -= DeltaSeconds;
            if (PostMoveRemainingSeconds <= 0.f)
            {
                bIsPostMovingForwardAfterSplineEnd = false;
            }
        }
    }

    if (!bIsFollowingSpline)
    {
        return;
    }

    USplineComponent* Spline = ActiveSpline;
    check(Spline != nullptr);

    // 進行距離を更新
    CurrentSplineDistance += ComputeMovementStep(DeltaSeconds);
    SplineProgressUpdatedDelegate.Broadcast(CurrentSplineDistance);

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

    // スプライン追従中の移動範囲制限（UpdatedComponent を半径内に拘束）
    if (APawn* ControlledPawn = GetPawn())
    {
        if (UPawnMovementComponent* Updated = ControlledPawn->GetMovementComponent())
        {
            const FVector ComponentLocation = Updated->GetActorLocation();

            // ピボット＋初期ローカルオフセット（Pawn のワールド回転を考慮して変換）を中心にする
            FVector ClampCenter = CurrentSplinePivotLocation;
            const FTransform PawnXform = ControlledPawn->GetActorTransform();
            const FVector WorldOffset = PawnXform.TransformVector(UpdatedComponentInitialLocalOffset);
            ClampCenter += WorldOffset;

            const FVector ToComponent = ComponentLocation - ClampCenter;

            const double DistanceFromPivot = ToComponent.Length();
            if (DistanceFromPivot > static_cast<double>(AllowedMoveRadius))
            {
                const FVector ClampedOffset = ToComponent.GetSafeNormal(UE_SMALL_NUMBER) * AllowedMoveRadius;
                const FVector ClampedLocation = ClampCenter + ClampedOffset;

                // スイープ無しでテレポートして範囲内へ収める（移動対象は UpdatedComponent）
                if (USceneComponent* UpdatedComp = Updated->UpdatedComponent)
                {
                    FHitResult TeleportHit;
                    UpdatedComp->SetWorldLocation(ClampedLocation, /*bSweep*/ false, /*OutHit*/ &TeleportHit, /*Teleport*/ ETeleportType::TeleportPhysics);
                }
            }
        }
    }
}

void AFlyingPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // InputMode を GameOnly に戻す
    {
        FInputModeGameOnly GameOnlyMode;
        SetInputMode(GameOnlyMode);
        bShowMouseCursor = false;
    }

    UpdatedComponentInitialLocalOffset = FVector::ZeroVector;

    if (USceneComponent* UpdatedComp = GetPawnUpdatedComponent())
    {
        // Pawn のルート（RootComponent）に対する UpdatedComponent のローカル位置
        UpdatedComponentInitialLocalOffset = UpdatedComp->GetRelativeLocation();
    }
}

void AFlyingPlayerController::OnUnPossess()
{
    Super::OnUnPossess();
    UpdatedComponentInitialLocalOffset = FVector::ZeroVector;
}

void AFlyingPlayerController::SetupInputComponent() 
{
	Super::SetupInputComponent();

	// Warn if no default mapping contexts are set
	if (DefaultMappingContexts.Num() == 0)
	{
#if WITH_EDITOR
		if (FModuleManager::Get().IsModuleLoaded("MessageLog"))
		{
			FMessageLog Log("PIE");
			Log.Warning(LOCTEXT("NoDefaultMappingContexts", "No DefaultMappingContexts set on FlyingPlayerController. Enhanced Input mappings will not be added."));
		}
#endif
		UE_LOG(LogFlyingPlayerController, Warning, TEXT("No DefaultMappingContexts set on %s"), *GetName());
	}

	// Add Input Mapping Contexts
	if (TObjectPtr<UEnhancedInputLocalPlayerSubsystem> Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		for (TObjectPtr<UInputMappingContext> CurrentContext : DefaultMappingContexts)
		{
			Subsystem->AddMappingContext(CurrentContext, 0);
		}
	}

	// Bind Enhanced Input actions
	if (TObjectPtr<UEnhancedInputComponent> EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (!MoveAction)
		{
#if WITH_EDITOR
			if (FModuleManager::Get().IsModuleLoaded("MessageLog"))
			{
				FMessageLog Log("PIE");
				Log.Warning(LOCTEXT("MoveActionNull", "MoveAction is null on FlyingPlayerController. Movement input will not be bound."));
			}
#endif
			UE_LOG(LogFlyingPlayerController, Warning, TEXT("MoveAction is null on %s"), *GetName());
		}
		else
		{
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFlyingPlayerController::HandleMoveInput);
		}
	}
}

void AFlyingPlayerController::HandleMoveInput(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
    DoMoveControlledPawn(MovementVector.X, MovementVector.Y);
}

void AFlyingPlayerController::DoMoveControlledPawn(const float Right, const float Up)
{
    // スプライン追従中のみ、プレイヤー操作を有効化
    if (!bIsFollowingSpline)
    {
        return;
    }

    APawn* ControlledPawn = GetPawn();
    if (nullptr == ControlledPawn)
    {
        return;
    }

    const FRotator Rotation = GetControlRotation();
    const FVector UpDirection = FRotationMatrix(Rotation).GetUnitAxis(EAxis::Z);
    const FVector RightDirection = FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y);

    ControlledPawn->AddMovementInput(UpDirection, Up);
    ControlledPawn->AddMovementInput(RightDirection, Right);
}

#undef LOCTEXT_NAMESPACE
