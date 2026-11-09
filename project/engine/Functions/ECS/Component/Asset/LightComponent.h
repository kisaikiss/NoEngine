#pragma once
#include "engine/Math/MathInclude.h"

namespace NoEngine::Component {
enum class DirectionalShadowSamples : uint32_t {
	kOneRay = 1,
	kFourRays = 4,
};

/// <summary>
/// 方向ライトのコンポーネント
/// </summary>
struct DirectionalLightComponent {
	Math::Color color;
	Math::Vector3 direction;
	float intensity;
	float shadowAngularRadiusDegrees = 0.f;
	DirectionalShadowSamples shadowSamples = DirectionalShadowSamples::kOneRay;
};
static_assert(sizeof(DirectionalLightComponent) == 40);

/// <summary>
/// ポイントライトのコンポーネント。TransformComponentと一緒に追加するとその場所から発光する
/// </summary>
struct PointLightComponent {
	Math::Color color;
	float intensity = 1.f;
	float radius = 1.f;
	float decay = 1.5f;
};

/// <summary>
/// スポットライトのコンポーネント。TransformComponentと一緒に追加するとその場所から発光する
/// </summary>
struct SpotLightComponent {
	Math::Color color;
	float intensity = 1.f;
	float distance = 5.f;
	float decay = 1.5f;
	float cosAngle = 0.8f;
	float cosFalloffStart = 1.f;
};

}