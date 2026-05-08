#pragma once
#include "../ISystem.h"
#include <engine/Math/Types/Vector2.h>

namespace NoEngine {
namespace ECS {
class CollisionResolution2DSystem :
    public ISystem {
public:
    void Update(Registry& registry, float deltaTime) override;

private:
    void Slide(Math::Vector2& velocity, const Math::Vector2& normal);
};
}
}