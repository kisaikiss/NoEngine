#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// 衝撃波システム
/// 衝撃波の拡大・フェードアウト・敵へのダメージ処理を行う
/// </summary>
class ShockwaveSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
};
