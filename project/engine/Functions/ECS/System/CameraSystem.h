#pragma once
#include "ISystem.h"

namespace NoEngine {
namespace ECS {
class CameraSystem :
    public ISystem {
public:
    CameraSystem(){ SetStopInGameStop(false); }
    void Update(Registry& registry, float deltaTime) override;
};
}
}

