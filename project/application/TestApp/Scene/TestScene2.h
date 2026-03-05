#pragma once
#include "engine/NoEngine.h"

class TestScene2 : public No::IScene {
public:
	void Setup() override;
private:
	void NotSystemUpdate() override;
};

