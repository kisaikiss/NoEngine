#pragma once
#include "engine/NoEngine.h"

struct HitBalloonComponent {

	/// 追従先の3Dエンティティ
	No::Entity sourceEntity{};

	/// アンカー基点からの微調整オフセット（スクリーン座標）
	No::Vector2 localOffset{ 0.f, 0.f };

	/// 吹き出しを出す基点の種類
	enum class AnchorType {
		TopRight,	// screenMax.x, screenMin.y （右上）
		Top,		// screenPosition.x, screenMin.y （真上中央）
		TopLeft,	// screenMin.x, screenMin.y （左上）
	};

	AnchorType anchorType = AnchorType::TopLeft;
};