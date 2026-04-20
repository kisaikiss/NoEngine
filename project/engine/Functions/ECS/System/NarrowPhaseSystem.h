#pragma once
#include "ISystem.h"
namespace NoEngine {
namespace ECS {
class NarrowPhaseSystem :
    public ISystem {
public:
    void Update(Registry& registry, float deltaTime) override;

};

}
}