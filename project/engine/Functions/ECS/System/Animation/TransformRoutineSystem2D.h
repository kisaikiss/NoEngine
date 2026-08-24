#pragma once
#include "../ISystem.h"

namespace NoEngine {
namespace ECS {
class TransformRoutineSystem2D : public ISystem {
public:
	void Update(Registry& registry, float deltaTime) override;
};
}
}