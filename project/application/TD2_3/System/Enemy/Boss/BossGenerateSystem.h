#pragma once
#include "engine/NoEngine.h"

class BossGenerateSystem : public No::ISystem {
public:
    void Update(No::Registry& registry, float deltaTime) override;
private:
    void GenerateBatBoss(No::Registry& registry);

    float timer_ = 0.f;
    bool isGenerate_ = false;
};

