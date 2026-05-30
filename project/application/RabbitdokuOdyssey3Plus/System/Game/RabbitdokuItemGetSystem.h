#pragma once
#include "engine/NoEngine.h"
class RabbitItemGetSystem :
    public No::ISystem {
public:
    void Update(No::Registry& registry, float deltaTime) override;
private:
    void GenerateLight(No::Registry& registry, No::Entity e);
    void GenerateSaveEffect(No::Registry& registry, No::Entity e);
    void GenerateStars(No::Registry& registry, No::Entity e);
    void GenerateHealedEffect(No::Registry& registry, No::Entity e);
    void GenerateClear(No::Registry& registry);

    bool GetIsTriggerUp();

    bool preIsPressUp_ = false;
    bool isPressUp_ = false;
};
