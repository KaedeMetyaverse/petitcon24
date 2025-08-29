#include "FlyingPawn.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Components/ArrowComponent.h"
#include "IHasHealth.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "FlyingPathMarkerActor.h"
#include "TimerManager.h"
#include "FlyingPawnCollisionComponent.h"
#include "FlyingPawnCollisionActor.h"

AFlyingPawn::AFlyingPawn()
{
    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;

    SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
    SkeletalMesh->SetupAttachment(Root);
    SkeletalMesh->SetRelativeRotation(FRotator(0, -90, 0));

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(Root);
    Camera->SetRelativeLocation(FVector(-360, 0, 20));
    Camera->SetRelativeRotation(FRotator(-10, 0, 0));

    MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
    MovementComponent->UpdatedComponent = SkeletalMesh;

    CollisionComponent = CreateDefaultSubobject<UFlyingPawnCollisionComponent>(TEXT("CollisionComponent"));
    CollisionComponent->SetupAttachment(SkeletalMesh);

    // コントローラーの回転に自動追従
    bUseControllerRotationYaw = true;
    bUseControllerRotationPitch = true;
    bUseControllerRotationRoll = true;

#if WITH_EDITORONLY_DATA
	ArrowComponent = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	if (ArrowComponent)
	{
		ArrowComponent->ArrowColor = FColor(150, 200, 255);
		ArrowComponent->SetupAttachment(Root);
		ArrowComponent->bIsScreenSizeScaled = true;
		ArrowComponent->SetSimulatePhysics(false);
	}
#endif // WITH_EDITORONLY_DATA
}

void AFlyingPawn::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    // 初期相対位置を記録
    if (ensureAlways(SkeletalMesh))
    {
        InitialSkeletalMeshRelativeLocation = SkeletalMesh->GetRelativeLocation();
    }

    if (ensureAlways(CollisionComponent))
    {
        DisableCollision();
    }

    // 補助アクタ側のヒットイベントにバインド
    BindCollisionHit();
}

void AFlyingPawn::OnPlayerStateChanged(APlayerState* NewPlayerState, APlayerState* OldPlayerState)
{
    Super::OnPlayerStateChanged(NewPlayerState, OldPlayerState);

    if (NewPlayerState != OldPlayerState)
    {
        if (OldPlayerState)
        {
            UnbindHealthChangedDelegate(OldPlayerState);
        }
    
        if (NewPlayerState)
        {
            BindHealthChangedDelegate(NewPlayerState);
        }
    }
}

void AFlyingPawn::ResetSkeletalMeshRelativeLocation()
{
    if (ensureAlways(SkeletalMesh))
    {
        SkeletalMesh->SetRelativeLocation(InitialSkeletalMeshRelativeLocation);
    }
}

float AFlyingPawn::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    float AppliedDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    AController* OwningController = GetController();
    if (ensureAlwaysMsgf(OwningController, TEXT("calling TakeDamage without OwningController is not expected")))
    {
        IHasHealth* Health = Cast<IHasHealth>(OwningController->PlayerState.Get());
        if (ensureAlways(Health))
        {
            if (!Health->IsDead())
            {
                Health->ApplyDamage(DamageAmount);
                AppliedDamage += DamageAmount;
            }
        }
    }

    return AppliedDamage;
}

void AFlyingPawn::OnPawnHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!OtherActor || OtherActor == this)
    {
        return;
    }

    UGameplayStatics::ApplyDamage(this, /*DamageAmount*/ 1.0f, /*EventInstigator*/ GetController(), /*DamageCauser*/ OtherActor, /*DamageTypeClass*/ nullptr);

    // 無敵化: 衝突判定は補助アクタ側へ委譲（プロファイル切替も補助アクタへ適用）
    if (ensureAlways(CollisionComponent))
    {
        DisableCollision();
    }
    GetWorldTimerManager().ClearTimer(InvincibilityTimerHandle);
    GetWorldTimerManager().SetTimer(InvincibilityTimerHandle, this, &AFlyingPawn::EndInvincibility, InvincibilitySeconds, /*bLoop*/ false);
}

void AFlyingPawn::EndInvincibility()
{
    // プロファイルを再適用（補助アクタの Sphere へ）
    if (ensureAlways(CollisionComponent))
    {
        EnableCollision();
    }
}

void AFlyingPawn::EnableCollision()
{
    GetWorldTimerManager().ClearTimer(InvincibilityTimerHandle);
    if (ensureAlways(CollisionComponent))
    {
        CollisionComponent->SetSpawnedCollisionProfile(TEXT("FlyingPawn"));
    }
}

void AFlyingPawn::DisableCollision()
{
    GetWorldTimerManager().ClearTimer(InvincibilityTimerHandle);
    if (ensureAlways(CollisionComponent))
    {
        CollisionComponent->SetSpawnedCollisionProfile(TEXT("Invincible"));
    }
}

void AFlyingPawn::SetLocationWithCollision(const FVector& NewLocation, bool bSweep)
{
    // Pawn 側へ適用
    SetActorLocation(NewLocation, /*bSweep*/ bSweep);

    // 補助アクタ側へも適用
    if (ensureAlways(CollisionComponent))
    {
        AFlyingPawnCollisionActor* CollisionActor = CollisionComponent->GetSpawnedActor();
        if (ensureAlways(CollisionActor))
        {
            CollisionActor->SetActorLocation(NewLocation, /*bSweep*/ bSweep);
        }
    }
}

void AFlyingPawn::BindHealthChangedDelegate(APlayerState* NewPlayerState)
{
    check(NewPlayerState);

    IHasHealth* Health = Cast<IHasHealth>(NewPlayerState);
    checkf(Health != nullptr, TEXT("BindHealthChangedDelegate requires PlayerState to implement IHasHealth"));

    check (!HealthChangedHandle.IsValid());
    HealthChangedHandle = Health->OnHealthChanged().AddUObject(this, &AFlyingPawn::HandleHealthChanged);
    HandleHealthChanged(Health->GetCurrentHP());
}

void AFlyingPawn::UnbindHealthChangedDelegate(APlayerState* OldPlayerState)
{
    check(OldPlayerState);

    IHasHealth* Health = Cast<IHasHealth>(OldPlayerState);
    checkf(Health != nullptr, TEXT("UnbindHealthChangedDelegate expects PlayerState to implement IHasHealth when handle is valid"));

    check(HealthChangedHandle.IsValid());
    Health->OnHealthChanged().Remove(HealthChangedHandle);
    HealthChangedHandle.Reset();
}

void AFlyingPawn::HandleHealthChanged(const int32 NewHP)
{
    // イベントを通知
    ReceiveHealthChanged(NewHP);
}

void AFlyingPawn::BindCollisionHit()
{
    if (!ensureAlways(CollisionComponent))
    {
        return;
    }

    AFlyingPawnCollisionActor* CollisionActor = CollisionComponent->GetSpawnedActor();
    if (!ensureAlways(CollisionActor))
    {
        return;
    }

    // バインド
    CollisionActor->OnActorHit.AddDynamic(this, &AFlyingPawn::OnPawnHit);
}
