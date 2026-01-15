#pragma once
#include "engine/NoEngine.h"

class CollisionSystem : public No::ISystem
{
public:
    void Update(No::Registry& registry, float deltaTime) override;
private:
    void UpdateCollider(No::Registry& registry);

    bool CheckSphereToSphere(const NoEngine::Vector3& center1, const NoEngine::Vector3& center2, const float radius1, const float radius2);
    bool CheckBoxToBox(const NoEngine::Vector3& center1, const NoEngine::Vector3& center2, const NoEngine::Vector3& size1, const NoEngine::Vector3& size2);
    bool CheckBoxToSphere(const NoEngine::Vector3& center1, const NoEngine::Vector3& center2, const NoEngine::Vector3& size, const float radius);
};