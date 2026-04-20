#pragma once
#include "ISystem.h"

namespace NoEngine {
namespace ECS {
class CollisionResolutionSystem :
    public ISystem {
public:
    void Update(Registry& registry, float deltaTime) override;
};
}
}

