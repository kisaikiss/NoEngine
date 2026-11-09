#pragma once
#include "engine/Math/MathInclude.h"
namespace NoEngine {
/// <summary>
/// GPUへポイントライトの情報を送るための構造体
/// </summary>
struct PointLightForGPU {
	Math::Color color;
	Math::Vector3 position;
	float intensity;
	float radius;
	float decay;
	float shadowRadius;
	uint32_t shadowSamples;
};
static_assert(sizeof(PointLightForGPU) == 48);

/// <summary>
/// GPUへスポットライトの情報を送るための構造体
/// </summary>
struct SpotLightForGPU {
	Math::Color color;
	Math::Vector3 position;
	float intensity;
	Math::Vector3 direction;
	float distance;
	float decay;
	float cosAngle;
	float cosFalloffStart;
	float shadowRadius;
	uint32_t shadowSamples;
};
static_assert(sizeof(SpotLightForGPU) == 68);



}
