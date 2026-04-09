#pragma once
#include "engine/Math/MathInclude.h"

namespace NoEngine::Component {
/// <summary>
/// 方向ライトのコンポーネント
/// </summary>
struct DirectionalLightComponent {
	Math::Color color;
	Math::Vector3 direction;
	float intensity;
};

/// <summary>
/// ポイントライトのコンポーネント。TransformComponentと一緒に追加するとその場所から発光する
/// </summary>
struct PointLightComponent {
	Math::Color color;
	float intensity;
	float radius;
	float decay;
};

/// <summary>
/// スポットライトのコンポーネント。TransformComponentと一緒に追加するとその場所から発光する
/// </summary>
struct SpotLightComponent {
	Math::Color color;
	float intensity;
	float distance;
	float decay;
	float cosAngle;
	float cosFalloffStart;
};

}