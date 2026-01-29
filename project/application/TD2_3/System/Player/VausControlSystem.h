#pragma once
#include "engine/NoEngine.h"

class VausControlSystem : public No::ISystem
{
public:
	void Update(No::Registry& registry, float deltaTime) override;
private:
	float CalculateMouseAngle();
	float CalculateStickAngle();
private:
	bool wasPress_ = false;
	bool isPress_ = false;
	float chargeTime_ = 0.0f;
	float power_ = 0.0f;
	enum class LastInput
	{
		Mouse,
		Stick
	};
	LastInput lastInput_ = LastInput::Mouse;
	// 前フレームのマウス角度（マウス移動検出用）
	float prevMouseAngle_ = 0.0f;
	float currentAngle_ = 0.0f;
	float mouseAngleOffset_ = 0.0f;

	float blendTime_ = 0.0f;
	float blendDuration_ = 0.12f;

	float blendFromAngle_ = 0.0f;
	bool isBlending_ = false;
};

