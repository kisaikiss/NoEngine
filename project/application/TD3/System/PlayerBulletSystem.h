#pragma once
#include "engine/NoEngine.h"
#include "../Component/PlayerBulletComponent.h"

/// <summary>
/// プレイヤーの弾丸システム
/// </summary>
class PlayerBulletSystem : public No::ISystem {
public:
	/// <summary>
	/// 更新処理
	/// </summary>
	void Update(No::Registry& registry, float deltaTime) override;

	/// <summary>
	/// カメラをセット（画面外判定用）
	/// </summary>
	void SetCamera(NoEngine::CameraBase* camera) { camera_ = camera; }

private:
	/// <summary>
	/// 指定ノードで弾を消滅すべきか判定する
	/// </summary>
	bool ShouldDestroyAtNode(No::Registry& registry, int nodeX, int nodeY, const No::Vector3& direction);
	
	/// <summary>
	/// 交差点（曲がり角）かどうかを判定する
	/// 進行方向と来た方向以外に接続がある場合に交差点とみなす
	/// </summary>
	bool IsIntersectionNode(No::Registry& registry, int nodeX, int nodeY, const No::Vector3& direction);
	
	/// <summary>
	/// 衝撃波エンティティを生成する
	/// </summary>
	void CreateShockwave(No::Registry& registry, const No::Vector3& position);
	
	/// <summary>
	/// ノード座標をハッシュ化する（訪問済み判定用）
	/// </summary>
	int HashNodeCoords(int x, int y) const;

	/// <summary>
	/// 画面外ループ処理
	/// </summary>
	void HandleScreenLooping(No::Registry& registry, No::Entity entity, PlayerBulletComponent* bullet, No::TransformComponent* transform);

	/// <summary>
	/// プレイヤーとの衝突判定（ループ後の弾のみ）
	/// </summary>
	void HandlePlayerCollision(No::Registry& registry, No::Entity bulletEntity, PlayerBulletComponent* bullet);

private:
	NoEngine::CameraBase* camera_ = nullptr;
};