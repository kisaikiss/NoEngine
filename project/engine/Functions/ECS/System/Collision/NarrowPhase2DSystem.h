#pragma once
#include "../ISystem.h"
#include "../../Event/CollisionEvents.h"
#include <engine/Functions/ECS/Registry.h>
namespace NoEngine {
namespace ECS{
class NarrowPhase2DSystem :
    public ISystem {
public:
    enum class TestAxis {
        Horizontal,
        Vertical,
        All
    };

    NarrowPhase2DSystem(TestAxis axis = TestAxis::All) : axis_(axis) {}
    void Update(Registry& registry, float deltaTime) override;

private:
    Math::ContactPosition ClassifyContact(const Math::Vector2& normal);

    TestAxis axis_;
};
}
}
