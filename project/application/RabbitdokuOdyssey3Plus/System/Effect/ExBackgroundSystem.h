#pragma once
#include "engine/NoEngine.h"
class ExBackgroundSystem :
    public No::ISystem {
public:
    ExBackgroundSystem() { SetStopInPause(false); }
    void Update(No::Registry& registry, float deltaTime) override;
private:
};