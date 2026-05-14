#pragma once
#include "engine/NoEngine.h"
class RabbitdokuStageEditSystem :
    public No::ISystem {
public:
    RabbitdokuStageEditSystem(){ SetStopInGameStop(false); }
    void Update(No::Registry& registry, float deltaTime) override;
private:
    void AddBlock(No::Registry& registry);
};

