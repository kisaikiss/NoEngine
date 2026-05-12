#pragma once
#include "engine/NoEngine.h"

class Camera2DChangeSystem : public No::ISystem {
public:
	Camera2DChangeSystem(){ SetStopInGameStop(false); SetStopInPause(false); }
	void Update(No::Registry& registry, float deltaTime) override;
};

