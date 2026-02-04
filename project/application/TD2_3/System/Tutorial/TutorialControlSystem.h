#pragma once
#include "engine/NoEngine.h"

class TutorialControlSystem : public No::ISystem
{
public:
	void Update(No::Registry& registry, float deltaTime) override;
private:
	float timer_ = 0.0f;

	float paddleShowTimer_ = 0.0f;
	float ballPongShowTimer_ = 0.0f;
	const float kShowTime_ = 3.0f;

	const float kMoveSpriteStartDuration_ = 1.5f;
	const float kMoveSpriteEndDuration_ = 1.5f;

	const float kBallPongSpriteStartDuration_ = 3.0f;
	const float kBallPongSpriteEndDuration_ = 3.0f;

	const float kEndPosX_ = 0.0f;
	const float kMiddlePosX_ = 640.0f;
	const float kStartPosX_ = 1280.0f;

};
