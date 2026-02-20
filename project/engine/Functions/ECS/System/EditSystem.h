#pragma once
#include "ISystem.h"
namespace NoEngine {
namespace ECS {
class EditSystem :
    public ISystem {
public:
    void Update(Registry& registry, float deltaTime) override;

};
}
}
