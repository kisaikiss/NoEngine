#pragma once

#include "engine/NoEngine.h"

class BossControlSystem : public No::ISystem
{
public:
    void Update(No::Registry& registry, float deltaTime) override;
private:
    float speed_ = 4.0f;
};

