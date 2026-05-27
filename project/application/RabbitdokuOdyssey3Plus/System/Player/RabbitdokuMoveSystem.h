#pragma once
#include "engine/NoEngine.h"
class RabbitdokuMoveSystem :
    public No::ISystem {
public:
    void Update(No::Registry& registry, float deltaTime) override;
private:
    void DeadMove(No::Registry& registry, No::Entity e, float deltaTime);
    void GenerateDeadSmoke(No::Registry& registry, No::Entity e);
};

