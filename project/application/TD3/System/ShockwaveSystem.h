#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// 衝撃波システム
/// 衝撃波の拡大・フェードアウト・敵へのダメージ処理を行う
/// </summary>
class ShockwaveSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;

	/// <summary>
	/// 敵撃破カウントのコールバックを設定
	/// </summary>
	void SetEnemyKillCallback(std::function<void()> callback) { onEnemyKilled_ = callback; }

private:
	std::function<void()> onEnemyKilled_;
};
