#pragma once
#include "engine/NoEngine.h"

class BatGirlControlSystem :
    public No::ISystem
{
public:
    BatGirlControlSystem();
    void Update(No::Registry& registry, float deltaTime) override;
    float timer_ = 10.0f;
    float idleTimer_  = 0.0f;
    uint32_t idleRandNum_ = 0;
    bool isWin_ = false;
    bool isLaughStart_ = false;
    bool isSetParent_ = false;
};

