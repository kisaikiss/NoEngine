#pragma once
#include "engine/NoEngine.h"

class OpeningScene : public No::IScene {
public:
	void Setup() override;
private:
	void AddSystems();

};