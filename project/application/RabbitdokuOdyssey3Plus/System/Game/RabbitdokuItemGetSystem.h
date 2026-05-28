#pragma once
#include "engine/NoEngine.h"
class RabbitItemGetSystem :
    public No::ISystem {
public:
    void Update(No::Registry& registry, float deltaTime) override;
private:
    void GenerateLight(No::Registry& registry, No::Entity e);
    void GenerateStars(No::Registry& registry, No::Entity e);
};
