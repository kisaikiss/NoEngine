#pragma once
#include "engine/NoEngine.h"

class HumanControlSystem :
    public No::ISystem
{
public:
    HumanControlSystem();
    void Update(No::Registry& registry, float deltaTime) override;
 
    const float kDuration_ = 1.5f;
    float timer_ = kDuration_;
    bool isChangePhase_ = false;
    float angle_ = 0.0f;
    float startAngle_ = 0.0f;
    NoEngine::Quaternion q_;
};

