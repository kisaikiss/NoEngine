#pragma once
#include "engine/NoEngine.h"
class SampleScene : public No::IScene {
public:
	void Setup() override;
private:
	void NotSystemUpdate() override;
};

