#include "FlyingPlayerCameraManager.h"
#include <cmath>

AFlyingPlayerCameraManager::AFlyingPlayerCameraManager()
{
    // 制限なし
    ViewPitchMin = std::nextafter(-180.f, 0.f);
    ViewPitchMax = std::nextafter(180.f, 0.f);
    ViewYawMin = std::nextafter(-180.f, 0.f);
    ViewYawMax = std::nextafter(180.f, 0.f);
    ViewRollMin = std::nextafter(-180.f, 0.f);
    ViewRollMax = std::nextafter(180.f, 0.f);
}
