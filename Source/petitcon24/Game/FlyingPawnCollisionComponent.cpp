#include "FlyingPawnCollisionComponent.h"
#include "FlyingPawnCollisionActor.h"
#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

UFlyingPawnCollisionComponent::UFlyingPawnCollisionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
	bWantsInitializeComponent         = true;

    SphereRadius = 80.f;
}

void UFlyingPawnCollisionComponent::InitializeComponent()
{
    Super::InitializeComponent();

    AActor* Owner = GetOwner();
    if (!ensureAlwaysMsgf(Owner, TEXT("UFlyingPawnCollisionComponent requires a valid owner")))
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!ensureAlways(World))
    {
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = Owner;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AFlyingPawnCollisionActor* FlyingPawnCollisionActor = World->SpawnActor<AFlyingPawnCollisionActor>(AFlyingPawnCollisionActor::StaticClass(), FTransform::Identity, SpawnParams);
    if (!ensureAlways(FlyingPawnCollisionActor))
    {
        return;
    }

    // 生成アクタの Root は Sphere を想定（コンストラクタで設定済み）
    USphereComponent* Sphere = FlyingPawnCollisionActor->GetSphereComponent();
    if (!ensureAlways(Sphere))
    {
        return;
    }

    Sphere->SetSphereRadius(SphereRadius, /*bUpdateOverlaps*/ true);

    // スポーン直後に現在のワールド変換へ即配置（以降の追従は Tick で行う）
    FlyingPawnCollisionActor->SetActorTransform(GetComponentTransform(), false);

    SpawnedCollisionActor = FlyingPawnCollisionActor;
}

void UFlyingPawnCollisionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (SpawnedCollisionActor)
    {
        AFlyingPawnCollisionActor* ActorToDestroy = SpawnedCollisionActor;
        SpawnedCollisionActor = nullptr;
        if (UWorld* World = GetWorld())
        {
            // 明示的に破棄
            ActorToDestroy->Destroy();
        }
    }

    Super::EndPlay(EndPlayReason);
}

void UFlyingPawnCollisionComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (ensureAlways(SpawnedCollisionActor))
    {
        const FTransform ComponentTransform = GetComponentTransform();
        SpawnedCollisionActor->SetActorTransform(ComponentTransform, /*bSweep*/ true);
    }
}

void UFlyingPawnCollisionComponent::SetSpawnedCollisionProfile(const FName InProfileName) const
{
    if (!ensureAlways(SpawnedCollisionActor))
    {
        return;
    }

    USphereComponent* Sphere = SpawnedCollisionActor->GetSphereComponent();
    if (!ensureAlways(Sphere))
    {
        return;
    }

    Sphere->SetCollisionProfileName(InProfileName);
}
