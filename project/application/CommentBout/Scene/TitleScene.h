#pragma once
#include "engine/NoEngine.h"

class TitleScene : public No::IScene {
public:
	void Setup() override;
private:
	void NotSystemUpdate() override;
};