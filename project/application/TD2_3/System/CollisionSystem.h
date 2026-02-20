#pragma once
#include "engine/NoEngine.h"

class CollisionSystem : public No::ISystem
{
public:
    void Update(No::Registry& registry, float deltaTime) override;
private:
    void UpdateCollider(No::Registry& registry);

    bool CheckSphereToSphere(const No::Vector3& center1, const No::Vector3& center2, const float radius1, const float radius2);
    bool CheckBoxToBox(const No::Vector3& center1, const No::Vector3& center2, const No::Vector3& size1, const No::Vector3& size2);
    bool CheckBoxToSphere(const No::Vector3& center1, const No::Vector3& center2, const No::Vector3& size, const float radius);
};