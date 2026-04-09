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
	float padding[2];
};



}