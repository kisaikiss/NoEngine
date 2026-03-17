#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// 吹き出しエフェクトの位置を毎フレーム更新するシステム
/// HitBalloonComponent が持つ sourceEntity の ProjectedColliderComponent から
/// screenMin / screenMax / screenPosition を取得し、Transform2D に反映する
/// LifetimeSystem より前に実行されること
/// </summary>
class HitBalloonSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
};