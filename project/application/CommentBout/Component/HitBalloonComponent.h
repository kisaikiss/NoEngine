#pragma once
#include "engine/NoEngine.h"

struct HitBalloonComponent {

	/// 追従先の ProjectedColliderComponent を持つ 3D エンティティ
	No::Entity sourceEntity{};

	/// アンカー基点からの微調整オフセット（スクリーン座標・ピクセル）
	No::Vector2 localOffset{ 0.f, 0.f };

	/// 草のスクリーン投影サイズに対する吹き出しサイズの比率
	/// x: 草のスクリーン幅  (screenMax.x - screenMin.x) に対する割合
	/// y: 草のスクリーン高さ (screenMax.y - screenMin.y) に対する割合
	/// 例: {0.8f, 0.5f} → 草の幅の80%、草の高さの50%
	/// {0, 0} にすると比率スケールを行わず GrassReactionComponent::effectSize の
	/// 固定ピクセルサイズで表示される
	No::Vector2 sizeRatio{ 0.f, 0.f };

	/// 吹き出しを出す基点の種類
	enum class AnchorType {
		TopRight,	// screenMax.x, screenMin.y （デフォルト: ドラクエ的右上）
		Top,		// screenPosition.x, screenMin.y （真上中央）
		TopLeft,	// screenMin.x, screenMin.y （左上）
	};

	AnchorType anchorType = AnchorType::TopRight;
};