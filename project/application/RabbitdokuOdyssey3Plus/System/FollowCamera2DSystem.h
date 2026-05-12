#pragma once
#include "engine/NoEngine.h"
class FollowCamera2DSystem :
    public No::ISystem {
public:
    void Update(No::Registry& registry, float deltaTime) override;
private:
    No::Entity FindRoom(No::Registry& registry, const No::Vector2& pos);
};

