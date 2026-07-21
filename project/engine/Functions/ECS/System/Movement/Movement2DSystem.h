#pragma once
#include "../ISystem.h"
namespace NoEngine {
namespace ECS {
class Movement2DSystem : public ISystem {
public:
	enum class MovementAxis {
		Horizontal,
		Vertical,
		All
	};

	Movement2DSystem(MovementAxis axis = MovementAxis::All) : axis_(axis) {}
	void Update(Registry& registry, float deltaTime) override;
private:
	MovementAxis axis_;
};
}
}