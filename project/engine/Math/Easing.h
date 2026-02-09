#pragma once
#include "Types/Vector3.h"
#include "Types/Quaternion.h"
#include <algorithm>
#include <numbers>

namespace
{
    constexpr float PI = std::numbers::pi_v<float>;
}

namespace NoEngine {
namespace Easing {
/// <summary>
/// 線形補間Vector3版
/// </summary>
/// <param name="start"></param>
/// <param name="end"></param>
/// <param name="t"></param>
/// <returns></returns>
Math::Vector3 Lerp(Math::Vector3 start, Math::Vector3 end, float t);

/// <summary>
/// 線形補間Quaternion版
/// </summary>
/// <param name="start"></param>
/// <param name="end"></param>
/// <param name="t"></param>
/// <returns></returns>
Math::Quaternion Lerp(Math::Quaternion start, Math::Quaternion end, float t);


template<typename T>
inline T Lerp(const T& start,const T& end, float t)
{
    return start + (end - start) * t;
}

template<typename T>
inline T EaseOutElastic(const T& a, const T& b, float t)
{
	const float c4 = (2 * PI) / 3;

	t = t == 0
		? 0
		: t == 1
		? 1
		: powf(2, -10 * t) * sinf((t * 10 - 0.75f) * c4) + 1;
	return Lerp(a, b, t);
}

template<typename T>
inline T EaseInOutBack(const T& a, const T& b, float t) {

	const float c1 = 1.70158f;
	const float c2 = c1 * 1.525f;

	float  time = t < 0.5f
		? (powf(2.0f * t, 2.0f) * ((c2 + 1.0f) * 2.0f * t - c2)) / 2.0f
		: (powf(2.0f * t - 2.0f, 2.0f) * ((c2 + 1.0f) * (t * 2.0f - 2.0f) + c2) + 2.0f) / 2.0f;

	return Lerp(a, b, time);
}

template<typename T>
inline T EaseOutCubic(const T& a, const T& b, float t)
{
	t = 1 - powf(1 - t, 3);
	return Lerp(a, b, t);
}

template<typename T>
inline T EaseOutCirc(const T& a, const T& b, float t)
{
	t = sqrt(1 - powf(t - 1, 2.0f));
	return Lerp(a, b, t);
}

template<typename T>
inline T EaseInOutSine(const T& a, const T& b, float t)
{
	t = -(cosf(PI * t) - 1.0f) / 2.0f;
	return Lerp(a, b, t);
}

template<typename T>
inline T EaseInExpo(const T& a, const T& b, float t)
{
	t = t == 0 ? 0 : powf(2.0f, 10 * t - 10);
	return Lerp(a, b, t);
}

template<typename T>
inline T EaseOutBack(const T& a, const T& b, float t)
{
	const float c1 = 1.70158f;
	const float c3 = c1 + 1.0f;

	t = 1 + c3 * powf(t - 1.0f, 3.0f) + c1 * pow(t - 1.0f, 2.0f);
	return Lerp(a, b, t);
}
}
}

