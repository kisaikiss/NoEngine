#pragma once
#include "engine/NoEngine.h"
class BigBadgeSystem :
    public No::ISystem {
public:
    void Update(No::Registry& registry, float deltaTime) override;
};