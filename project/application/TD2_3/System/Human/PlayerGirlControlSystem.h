#pragma once

#include "engine/NoEngine.h"
class PlayerGirlControlSystem :
    public No::ISystem
{
public:
    PlayerGirlControlSystem();
    void Update(No::Registry& registry, float deltaTime) override;
private:
    float timer_ = 0.0f;
};

