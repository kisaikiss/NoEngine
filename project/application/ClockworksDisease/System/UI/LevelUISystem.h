#pragma once
#include "engine/NoEngine.h"

struct LevelGaugeTag{};

struct LevelNumberTag{};

class LevelUISystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
private:
};
