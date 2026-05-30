#pragma once
#include "engine/NoEngine.h"
class ClearSystem :
    public No::ISystem {
public:
    ClearSystem() { SetStopInPause(false); }
    void Update(No::Registry& registry, float deltaTime) override;
private:
    void GenerateEffect(No::Registry& registry);
};