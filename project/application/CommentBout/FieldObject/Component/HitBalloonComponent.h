#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// フィールドオブジェクト反応で表示する吹き出し情報。
/// </summary>
struct HitBalloonComponent {
	No::Entity sourceEntity{};
	No::Vector2 localOffset{ 0.f, 0.f };
	No::Vector2 sizeRatio{ 0.f, 0.f };

	enum class AnchorType {
		TopRight,
		Top,
		TopLeft,
	};

	AnchorType anchorType = AnchorType::TopRight;
};
