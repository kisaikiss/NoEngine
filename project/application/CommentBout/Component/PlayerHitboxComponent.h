#pragma once
#include "engine/NoEngine.h"

struct PlayerHitboxComponent {
    No::Entity playerEntity = No::nullEntity;

    bool useCameraGateForPlayerHit = true;
    float cameraGateNear = 0.0f;
    float cameraGateDepth = 0.5f;
    float cameraGateHalfWidth = 0.6f;
    float cameraGateHalfHeight = 0.32f;
};

