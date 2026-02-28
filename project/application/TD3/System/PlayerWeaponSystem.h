#pragma once
#include "engine/NoEngine.h"
#include "../Component/PlayerComponent.h"
#include "../Component/GridCellComponent.h"
#include "../GameTag.h"

/// <summary>
/// プレイヤー武器システム
/// 弾丸発射とImGuiでの弾薬パラメータ表示を担当する。
/// </summary>
class PlayerWeaponSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;

private:
	/// <summary>
	/// 弾丸発射処理（スペースキー）
	/// </summary>
	void HandleBulletFire(
		PlayerComponent* player,
		No::Registry& registry,
		const No::Vector3 playerPosition,				// 値渡し：ダングリングリファレンス防止
		const NoEngine::Math::Quaternion playerRotation	// 値渡し：ダングリングリファレンス防止
	);

	/// <summary>
	/// 方向から正規化ベクトルを取得
	/// </summary>
	No::Vector3 DirectionToVector(Direction dir);

#ifdef USE_IMGUI
	void DebugUI(PlayerComponent* player);
#endif
};