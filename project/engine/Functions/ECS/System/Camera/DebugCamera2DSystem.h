#pragma once
#include "../ISystem.h"
namespace NoEngine {
namespace ECS {

class DebugCamera2DSystem :
    public ISystem {
public:
    DebugCamera2DSystem() { SetStopInGameStop(false); SetStopInPause(false); }
    void Update(Registry& registry, float deltaTime) override;
};


}
}