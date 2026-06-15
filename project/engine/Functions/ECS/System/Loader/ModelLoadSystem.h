#pragma once
#include "../ISystem.h"
namespace NoEngine {
namespace ECS {
class ModelLoadSystem :
	public ISystem {
public:
	ModelLoadSystem() { SetStopInGameStop(false); SetStopInPause(false); }
	void Update(Registry& registry, float deltaTime) override;
};
}
}