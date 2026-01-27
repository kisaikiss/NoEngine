#pragma once

struct VausStateComponent
{
	float currentRingRadius = 4.85f;
	float theta = 0.0f;
	float chargePower = 0.0f; //0.0f〜1.0f
	uint32_t hp = 5;

	NoEngine::Vector3 prevPosition = NoEngine::Vector3::ZERO;
	bool hasReleasedMovement = false;
	float  releaseTime = 0.0f;
	NoEngine::Vector3 releaseVelocity = NoEngine::Vector3::ZERO;
	NoEngine::Vector3 releaseVelocityAccum = NoEngine::Vector3::ZERO;
	static constexpr float kPower = 15.0f;
};