#pragma once

struct BallStateComponent
{
	bool landed = true;
	bool isOut = false;


	static constexpr float kSlowDuration = 3.0f;
	static constexpr float kSlowRcp = 1.0f / kSlowDuration;

	static inline float ballSlowFactor = 1.0f;
	static inline float ballSlowtime = kSlowDuration;
};
