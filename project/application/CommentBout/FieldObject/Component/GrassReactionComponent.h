#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// 草オブジェクトの反応状態とヒット演出設定。
/// FieldObject 層で管理する。
/// </summary>
struct GrassReactionComponent {
	bool wasColliding = false;

	// エフェクト共通
	float effectLifetime = 0.3f;
	int effectLayer = 40;

	// アンカー（草のスクリーン右上端）からの微調整オフセット（ピクセル）
	No::Vector2 effectOffset{ 0.0f, 0.0f };

	// 吹き出しの固定ピクセルサイズ（sizeRatio が {0,0} のときに使用）
	No::Vector2 effectSize{ 120.0f, 60.0f };

	// 草のスクリーン投影サイズに対する吹き出しサイズ比率
	No::Vector2 sizeRatio{ 1.f, 0.8f };
};
