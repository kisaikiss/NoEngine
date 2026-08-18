#pragma once
#include "engine/NoEngine.h"

// GameProgressComponent::elapsedTimeを毎フレーム加算するだけのSystem。
// デフォルトのSetStopInPause/SetStopInGameStopのため、ポーズ中・ゲーム停止中は加算されない。
class GameTimerSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
};