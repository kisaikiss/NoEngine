#pragma once

struct VausStateComponent
{
	float currentRingRadius = 4.85f;
	float theta = 0.0f;
	float chargePower = 0.0f; //0.0f〜1.0f
	uint32_t hp = 5;

	float releaseTime = 0.0f;
	
	NoEngine::Vector3 prevPosition = NoEngine::Vector3::ZERO;
	NoEngine::Vector3 currentVelocity = NoEngine::Vector3::ZERO;

	bool isReleasing = false;
	static constexpr float kPower = 0.5f;
};