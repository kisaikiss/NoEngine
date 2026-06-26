#pragma once
#include "../ISystem.h"

namespace NoEngine {
namespace ECS {
class SpriteAnimationSystem : public ISystem {
public:
	void Update(Registry& registry, float deltaTime) override;
};
}
}