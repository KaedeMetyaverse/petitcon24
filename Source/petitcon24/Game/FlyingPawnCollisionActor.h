#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FlyingPawnCollisionActor.generated.h"

class USphereComponent;

UCLASS()
class PETITCON24_API AFlyingPawnCollisionActor : public AActor
{
    GENERATED_BODY()

public:
    AFlyingPawnCollisionActor();

    // 取得用アクセサ
    class USphereComponent* GetSphereComponent() const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
    TObjectPtr<USphereComponent> SphereComponent;
};
