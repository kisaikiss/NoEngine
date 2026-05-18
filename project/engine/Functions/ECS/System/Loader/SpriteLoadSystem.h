#pragma once
#include "../ISystem.h"
namespace NoEngine {
namespace ECS {

class SpriteLoadSystem :
	public ISystem {
public:
	SpriteLoadSystem() { SetStopInGameStop(false); SetStopInPause(false); }
	void Update(Registry& registry, float deltaTime) override;
};


}

}