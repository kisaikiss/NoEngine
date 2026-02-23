#pragma once
#include "engine/NoEngine.h"
#include "../Component/PlayerComponent.h"
#include "../Component/GridCellComponent.h"
#include "../GameTag.h"

/// <summary>
/// プレイヤー武器システム
/// 弾丸発射とImGuiでの弾薬パラメータ表示を担当する
/// </summary>
class PlayerWeaponSystem : public No::ISystem {
public:
	/// <summary>
	/// 更新処理
	/// </summary>
	void Update(No::Registry& registry, float deltaTime) override;

private:
	// ========== 弾丸発射 ==========

	/// <summary>
	/// 弾丸発射処理
	/// スペースキーで弾丸を発射する
	/// </summary>
	void HandleBulletFire(
		PlayerComponent* player,
		No::Registry& registry,
		const No::Vector3& playerPosition
	);

	/// <summary>
	/// 方向から正規化されたベクトルを取得
	/// </summary>
	No::Vector3 DirectionToVector(Direction dir);

	/// <summary>
	/// 指定座標のグリッドセルを取得する
	/// 見つからない場合は nullptr を返す
	/// </summary>
	GridCellComponent* GetGridCell(
		No::Registry& registry,
		int x, int y
	);

	/// <summary>
	/// 指定方向への接続があるか返す
	/// </summary>
	bool HasConnection(
		const GridCellComponent* cell,
		Direction dir
	);

	// ========== デバッグ UI ==========
#ifdef USE_IMGUI
	void DebugUI(PlayerComponent* player);
#endif
};
