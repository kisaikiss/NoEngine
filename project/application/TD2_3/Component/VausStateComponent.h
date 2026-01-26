#pragma once

struct VausStateComponent
{
	float currentRingRadius = 4.85f;
	float theta = 0.0f;
	float chargePower = 0.0f; //0.0f〜1.0f
	uint32_t hp = 5;

	static constexpr float kPower = 15.0f;
};