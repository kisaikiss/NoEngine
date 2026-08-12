#pragma once
#include "engine/NoEngine.h"
class BoxColliderUpdateSystem :
    public No::ISystem {
public:
    BoxColliderUpdateSystem() { SetStopInGameStop(false); SetStopInPause(false);}
    void Update(No::Registry& registry, float deltaTime) override;
};

