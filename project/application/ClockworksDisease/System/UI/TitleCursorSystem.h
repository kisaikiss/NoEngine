#pragma once
#include "engine/NoEngine.h"

class TitleCursorSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
private:

};