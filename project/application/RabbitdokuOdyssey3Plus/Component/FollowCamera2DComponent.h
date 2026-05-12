#pragma once
#include"engine/NoEngine.h"

struct FollowCamera2DComponent {
    uint32_t currentRoom;
    No::Vector2 lerpPosition{};
};