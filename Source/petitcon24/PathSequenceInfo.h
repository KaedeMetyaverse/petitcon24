#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "PathActor.h"
#include "PathSequenceInfo.generated.h"

// レベルに配置して経路（PathActor の列）をデータとして保持するための情報アクター
UCLASS()
class PETITCON24_API APathSequenceInfo : public AInfo
{
    GENERATED_BODY()

public:
    // レベル単位のデータなので EditInstanceOnly
    UPROPERTY(EditInstanceOnly, Category = "Path Follow")
    TArray<TObjectPtr<APathActor>> PathActorsSequence;
};
