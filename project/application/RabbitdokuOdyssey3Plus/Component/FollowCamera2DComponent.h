#pragma once
#include"engine/NoEngine.h"

struct FollowCamera2DComponent {
    uint32_t currentRoom;
    bool isTransition = false;
    float t = 0.0f;
    float transitionSpeed = 3.0f;
    No::Vector2 targetPos;
};