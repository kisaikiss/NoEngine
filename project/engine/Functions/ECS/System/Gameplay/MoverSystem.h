#pragma once
#include "../ISystem.h"

namespace NoEngine::ECS {
class MoverSystem : public ISystem {
public:
	void Update(Registry& registry, float deltaTime) override;
};
}
