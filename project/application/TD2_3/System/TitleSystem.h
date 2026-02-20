#pragma once
#include "engine/NoEngine.h"

class TitleSystem : public No::ISystem
{
public:
	void Update(No::Registry& registry, float deltaTime) override;
private:
	bool isChangeScene_ = false;
	float time_ = 0.f;
};