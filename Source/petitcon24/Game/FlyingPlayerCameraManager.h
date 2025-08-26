#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "FlyingPlayerCameraManager.generated.h"

UCLASS()
class PETITCON24_API AFlyingPlayerCameraManager : public APlayerCameraManager
{
    GENERATED_BODY()

public:
    AFlyingPlayerCameraManager();
};
