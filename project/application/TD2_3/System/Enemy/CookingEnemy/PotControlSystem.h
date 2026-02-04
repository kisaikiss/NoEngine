#pragma once
#include "engine/NoEngine.h"

class PotControlSystem : public No::ISystem {
public:
    void Update(No::Registry& registry, float deltaTime) override;
private:
    void GenerateUpdate(No::Registry& registry, No::Entity entity, float deltaTime);
    void MoveUpdate(No::Registry& registry, No::Entity entity, float deltaTime);
    void DeadUpdate(No::Registry& registry, No::Entity entity, float deltaTime);

    void Shoot(No::Registry& registry, No::TransformComponent* transform, const NoEngine::Vector3& target);
    void GenerateSmokeEffect(No::Registry& registry, NoEngine::Vector3 position);
    float speed_ = 4.0f;

};

