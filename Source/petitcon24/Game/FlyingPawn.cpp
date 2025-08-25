#include "FlyingPawn.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Components/ArrowComponent.h"
#include "HealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "FlyingPathMarkerActor.h"
#include "TimerManager.h"

AFlyingPawn::AFlyingPawn()
{
    RootSphere = CreateDefaultSubobject<USphereComponent>(TEXT("RootSphere"));
    RootSphere->SetCollisionProfileName(TEXT("FlyingPawn"));
    RootComponent = RootSphere;

    SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
    SkeletalMesh->SetupAttachment(RootSphere);
    SkeletalMesh->SetRelativeRotation(FRotator(0, -90, 0));

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(RootSphere);
    Camera->SetRelativeLocation(FVector(-360, 0, 20));
    Camera->SetRelativeRotation(FRotator(-10, 0, 0));

    MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
    MovementComponent->UpdatedComponent = SkeletalMesh;

    HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

    // コントローラーの回転に自動追従
    bUseControllerRotationYaw = true;
    bUseControllerRotationPitch = true;
    bUseControllerRotationRoll = true;

#if WITH_EDITORONLY_DATA
	ArrowComponent = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	if (ArrowComponent)
	{
		ArrowComponent->ArrowColor = FColor(150, 200, 255);
		ArrowComponent->SetupAttachment(RootSphere);
		ArrowComponent->bIsScreenSizeScaled = true;
		ArrowComponent->SetSimulatePhysics(false);
	}
#endif // WITH_EDITORONLY_DATA
}

void AFlyingPawn::BeginPlay()
{
    Super::BeginPlay();

    // アクター単位の Hit（物理衝突）を検知
    OnActorHit.AddDynamic(this, &AFlyingPawn::OnPawnHit);
}

float AFlyingPawn::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    float AppliedDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    if (!HealthComponent->IsDead())
    {
        HealthComponent->ApplyDamage(DamageAmount);
        AppliedDamage += DamageAmount;
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

    // 無敵化: RootSphere のコリジョンを一時的に無効化し、3秒後に復帰
    RootSphere->SetCollisionProfileName(TEXT("Invincible"));
    GetWorldTimerManager().ClearTimer(InvincibilityTimerHandle);
    GetWorldTimerManager().SetTimer(InvincibilityTimerHandle, this, &AFlyingPawn::EndInvincibility, InvincibilitySeconds, /*bLoop*/ false);
}

void AFlyingPawn::EndInvincibility()
{
    // プロファイルを再適用
    RootSphere->SetCollisionProfileName(TEXT("FlyingPawn"));
}
