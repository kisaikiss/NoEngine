#pragma once
#include "engine/NoEngine.h"

class EngineTestScene : public No::IScene {
public:
	void Setup() override;
private:
	void NotSystemUpdate() override;
};

