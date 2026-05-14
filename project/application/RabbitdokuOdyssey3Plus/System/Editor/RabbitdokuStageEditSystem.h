#pragma once
#include "engine/NoEngine.h"
class RabbitdokuStageEditSystem :
    public No::ISystem {
public:
    RabbitdokuStageEditSystem() { SetStopInGameStop(false); SetStopInGameStop(false); }
    void Update(No::Registry& registry, float deltaTime) override;
private:

    No::Vector2 gridSize_ = No::Vector2(64.f,64.f);

    void AddBlock(No::Registry& registry);
    void DeleteBlock(No::Registry& registry);
    No::Vector2 GetGridPosition(const No::Vector2& position);

    void DrawEditWindow();
};

