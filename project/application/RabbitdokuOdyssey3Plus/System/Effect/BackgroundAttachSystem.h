#pragma once
#include "engine/NoEngine.h"
class BackgroundAttachSystem :
    public No::ISystem {
public:
    BackgroundAttachSystem() { SetStopInPause(false); }
    void Update(No::Registry& registry, float deltaTime) override;
private:
};