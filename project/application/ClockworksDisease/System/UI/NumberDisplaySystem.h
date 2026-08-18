#pragma once
#include "engine/NoEngine.h"

// NumberDisplayComponentのvalueを桁ごとに分解し、digitEntitiesのSpriteComponent::uvを更新する。
// 数字画像は左から0,1,2...9の順に10等分されている前提（levelNumbers.png）。
class NumberDisplaySystem : public No::ISystem {
public:
	NumberDisplaySystem() { SetStopInGameStop(false); SetStopInPause(false); }
	void Update(No::Registry& registry, float deltaTime) override;
};