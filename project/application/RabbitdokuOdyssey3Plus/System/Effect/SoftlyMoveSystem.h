#pragma once
#include "engine/NoEngine.h"
class SoftlyMoveSystem :
    public No::ISystem {
public:
    SoftlyMoveSystem() {}
    void Update(No::Registry& registry, float deltaTime) override;
private:
};