#pragma once
#include "../ISystem.h"

namespace NoEngine {
namespace ECS {
class Floating2DSystem : public ISystem {
public:
	void Update(Registry& registry, float deltaTime) override;

};
}
}