#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// 吹き出しエフェクトの位置・サイズを毎フレーム更新するFieldObject層System。
/// </summary>
class HitBalloonSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
};
