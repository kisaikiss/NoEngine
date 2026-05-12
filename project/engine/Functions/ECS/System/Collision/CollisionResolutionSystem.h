#pragma once
#include "../ISystem.h"

namespace NoEngine {
namespace ECS {
class CollisionResolutionSystem :
    public ISystem {
public:
    void Update(Registry& registry, float deltaTime) override;

private:
    void Slide(Math::Vector3& velocity, const Math::Vector3& normal);
};
}
}

