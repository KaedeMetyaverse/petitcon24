#pragma once

#include "CoreMinimal.h"

// 体力更新通知の共通デリゲート型
DECLARE_MULTICAST_DELEGATE_OneParam(FOnHealthChangedDelegate, int32 /*NewHP*/);
