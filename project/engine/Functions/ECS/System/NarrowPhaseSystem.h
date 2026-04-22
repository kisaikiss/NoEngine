#pragma once
#include "ISystem.h"
#include "../Event/CollisionEvents.h"
namespace NoEngine {
namespace ECS {
class NarrowPhaseSystem :
    public ISystem {
public:
    void Update(Registry& registry, float deltaTime) override;

private:
	Math::ContactPosition ClassifyContact(const Math::Vector3& normal);
};

}
}