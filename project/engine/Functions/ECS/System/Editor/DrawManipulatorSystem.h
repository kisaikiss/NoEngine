#pragma once
#include "../ISystem.h"
namespace NoEngine {
namespace ECS {

class DrawManipulatorSystem :
	public ISystem {
public:
	DrawManipulatorSystem() { SetStopInGameStop(false); SetStopInPause(false); }
	void Update(Registry& registry, float deltaTime) override;
private:
	bool isActive_ = false;
	bool isActivePreFrame_ = false;

	void Manipulate3D(Registry& registry);
	void Manipulate2D(Registry& registry);
};

}
}

