#pragma once
#include "engine/NoEngine.h"

class TestScene3 : public No::IScene {
public:
	void Setup() override;
private:
	void NotSystemUpdate() override;

	void DrawCollisionImGui();
};

