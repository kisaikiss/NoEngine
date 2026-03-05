#pragma once
#include "engine/NoEngine.h"

class TestScene : public No::IScene {
public:
	void Setup() override;
private:
	void NotSystemUpdate() override;
};

