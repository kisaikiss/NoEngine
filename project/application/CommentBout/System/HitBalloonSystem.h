#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// 吹き出しエフェクトの位置・サイズを毎フレーム更新するシステム
///
///【位置】sourceEntity の ProjectedColliderComponent から screenMin / screenMax を取得し AnchorType に応じて transform2D->translate を更新
///
/// 【サイズ】sizeRatio が {0,0} 以外の場合、
/// 草のスクリーン投影 AABB サイズ (screenMax - screenMin) に対する比率で transform2D->scale を更新する（カメラ距離・角度に自動追従）
/// sizeRatio が {0,0} なら GrassReactionSystem がセットした固定ピクセルを維持
/// </summary>
class HitBalloonSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
};