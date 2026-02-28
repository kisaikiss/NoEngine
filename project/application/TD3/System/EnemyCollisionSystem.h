#pragma once
#include "engine/NoEngine.h"
#include "../Component/HealthComponent.h"

/// <summary>
/// 敵衝突システム
/// プレイヤーと敵の衝突判定を行い、接触時に敵を死亡させ、プレイヤーに1ダメージを与える。
/// 衝突判定自体はCollisionSystemが担当してこのSystemは結果（isColliedフラグ）を参照してダメージ処理を行う。
/// </summary>
class EnemyCollisionSystem : public No::ISystem {
public:
	/// <summary>
	/// 更新処理
	/// </summary>
	void Update(No::Registry& registry, float deltaTime) override;

	/// <summary>
	/// 敵撃破カウントのコールバックを設定
	/// </summary>
	void SetEnemyKillCallback(std::function<void()> callback) { onEnemyKilled_ = callback; }

private:
	std::function<void()> onEnemyKilled_;

#ifdef USE_IMGUI
	/// <summary>
	/// プレイヤーの HP 表示・無敵フラグ操作ウィンドウ
	/// </summary>
	void DebugUI(HealthComponent* playerHealth);
#endif
};