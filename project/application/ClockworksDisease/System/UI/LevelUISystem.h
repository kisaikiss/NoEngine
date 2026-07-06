#pragma once
#include "engine/NoEngine.h"

struct LevelGaugeTag{};

class LevelUISystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
private:
};
