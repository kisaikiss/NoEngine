#pragma once
#include "engine/NoEngine.h"
class SmokeEffectSystem :
    public No::ISystem {
public:
    SmokeEffectSystem() {}
    void Update(No::Registry& registry, float deltaTime) override;
private:
};

