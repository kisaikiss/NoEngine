#pragma once
#include "../ISystem.h"

namespace NoEngine {
namespace ECS {
class Camera2DSystem :
    public ISystem {
public:
    Camera2DSystem() { SetStopInGameStop(false); SetStopInPause(false); }
    void Update(Registry& registry, float deltaTime) override;
};
}
}

