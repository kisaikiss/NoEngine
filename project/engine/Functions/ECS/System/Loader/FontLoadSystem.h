#pragma once
#include "../ISystem.h"

namespace NoEngine::ECS {
class FontLoadSystem : public ISystem {
public:
	FontLoadSystem() { SetStopInGameStop(false); SetStopInPause(false); }
	void Update(Registry& registry, float deltaTime) override;
};
}
