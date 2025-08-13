#include "FlyingPawn.h"

AFlyingPawn::AFlyingPawn()
{
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;

    SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
    SkeletalMesh->SetupAttachment(SceneRoot);
    SkeletalMesh->SetRelativeRotation(FRotator(0, -90, 0));

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SceneRoot);
    Camera->SetRelativeLocation(FVector(-360, 0, 20));
    Camera->SetRelativeRotation(FRotator(-10, 0, 0));
}
