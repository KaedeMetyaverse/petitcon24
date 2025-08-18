#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TitleWidget.h"
#include "TitlePlayerController.generated.h"

UCLASS()
class PETITCON24_API ATitlePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ATitlePlayerController();

protected:
	virtual void BeginPlay() override;

private:
	TWeakObjectPtr<UTitleWidget> SpawnedTitleWidget;
};
