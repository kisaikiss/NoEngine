#pragma once

#include "engine/NoEngine.h"
class ResultPlayerGirlControlSystem :
    public No::ISystem
{
public:
    void Update(No::Registry& registry, float deltaTime) override;
private:
    bool isGameOverAnimationStart_ = false;
};
