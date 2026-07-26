#pragma once
#include "engine/NoEngine.h"
class TerrainLoadSystem :
    public No::ISystem {
public:
    TerrainLoadSystem() { SetStopInGameStop(false); SetStopInPause(false); }
    void Update(No::Registry& registry, float deltaTime) override;
};