#pragma once
#include "engine/NoEngine.h"

class BatGirlControlSystem :
    public No::ISystem
{
public:
    BatGirlControlSystem();
    void Update(No::Registry& registry, float deltaTime) override;
    float timer_ = 0.0f;
};

