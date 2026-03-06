#pragma once
#include "engine/NoEngine.h"
#include "../Component/PlayerBulletComponent.h"

// 前方宣言
struct DeathFlag;

/// <summary>
/// プレイヤーの弾丸システム
/// </summary>
class PlayerBulletSystem : public No::ISystem {
public:
	/// <summary>
	/// 更新処理
	/// </summary>
	void Update(NoEngine::ECS::Registry& registry, float deltaTime) override;

	/// <summary>
	/// 敵撃破カウントのコールバックを設定
	/// </summary>
	void SetEnemyKillCallback(std::function<void()> callback) { onEnemyKilled_ = callback; }

private:
	/// <summary>
	/// 指定ノードで弾を消滅すべきか判定する
	/// </summary>
	bool ShouldDestroyAtNode(NoEngine::ECS::Registry& registry, int nodeX, int nodeY, const No::Vector3& direction);

	/// <summary>
	/// 交差点（曲がり角）かどうかを判定する
	/// 進行方向と来た方向以外に接続がある場合に交差点とみなす
	/// </summary>
	bool IsIntersectionNode(NoEngine::ECS::Registry& registry, int nodeX, int nodeY, const No::Vector3& direction);

	/// <summary>
	/// 衝撃波エンティティを生成する
	/// </summary>
	void CreateShockwave(NoEngine::ECS::Registry& registry, const No::Vector3& position);

	/// <summary>
	/// ノード座標をハッシュ化する（訪問済み判定用）
	/// </summary>
	int HashNodeCoords(int x, int y) const;

	/// <summary>
	/// 画面外ループ処理（新：削除＆生成方式）
	/// </summary>
	void HandleScreenLooping(NoEngine::ECS::Registry& registry, No::Entity entity, PlayerBulletComponent* bullet, No::TransformComponent* transform, DeathFlag* deathFlag);

	/// <summary>
	/// ループ弾を生成
	/// </summary>
	void CreateLoopedBullet(NoEngine::ECS::Registry& registry, const No::Vector3& loopedPosition, const PlayerBulletComponent originalBullet);

	/// <summary>
	/// プレイヤーとの衝突判定（ループ弾のみ）
	/// </summary>
	void HandlePlayerCollision(NoEngine::ECS::Registry& registry, No::Entity bulletEntity, PlayerBulletComponent* bullet);

	/// <summary>
	/// ループ座標を計算
	/// </summary>
	No::Vector3 CalculateLoopedPosition(NoEngine::ECS::Registry& registry, const No::Vector3& currentPosition, float offset);

private:
	std::function<void()> onEnemyKilled_;
};