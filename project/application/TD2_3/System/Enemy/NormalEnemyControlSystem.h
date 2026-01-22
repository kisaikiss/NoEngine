#pragma once
#include "engine/NoEngine.h"
#include"engine/Math/Types/Vector3.h"
class NormalEnemyControlSystem : public No::ISystem
{
public:
    void Update(No::Registry& registry, float deltaTime) override;
private:
    NoEngine::Vector3 velocity_ = {0.0f };
};

