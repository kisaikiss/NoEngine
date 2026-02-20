#pragma once
#include "engine/NoEngine.h"

class ChefControlSystem :
    public No::ISystem
{
public:
    ChefControlSystem();
    void Update(No::Registry& registry, float deltaTime) override;
    float timer_ = 10.0f;
    float idleTimer_ = 0.0f;
    bool isWin_ = false;
    bool isAmoreStart_ = false;
    bool isBallHit_ = false;
    std::vector<std::string> loseVoice_;
};

