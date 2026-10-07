#pragma once
#include "engine/NoEngine.h"

struct SceneChangeTimerComponent {
	float sceneChangeTime = 10.0f;
};

class SceneChangeTimerSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
private:
	float sceneChangeTimer_ = 0.0f;
};

