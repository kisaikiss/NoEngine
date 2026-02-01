#pragma once

struct VausStateComponent
{
	float currentRingRadius = 4.85f;
	float theta = 0.0f;
	float chargePower = 0.0f; //0.0f〜1.0f
	float widthScale = 1.0f;
	float releaseTime = 0.0f;
	
	NoEngine::Vector3 prevPosition = NoEngine::Vector3::ZERO;
	NoEngine::Vector3 currentVelocity = NoEngine::Vector3::ZERO;

	float playerDamageCooldown = 0.0f;
	bool isHerted = false;

	bool isReleasing = false;
	static constexpr float kBaseWidth = 3.5f;
	static constexpr float kPower = 150.0f;
	static constexpr float kDamageCooldownTime = 2.0f;
};