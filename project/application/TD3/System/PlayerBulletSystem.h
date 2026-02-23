#pragma once
#include "engine/NoEngine.h"
#include "../Component/PlayerBulletComponent.h"
#include "../Component/GridCellComponent.h"

/// <summary>
/// プレイヤー弾丸システム
/// 弾丸の移動・消滅判定を管理する
/// </summary>
class PlayerBulletSystem : public No::ISystem {
public:
	/// <summary>
	/// 更新処理
	/// </summary>
	void Update(No::Registry& registry, float deltaTime) override;

private:
	/// <summary>
	/// 指定ノードで弾丸を消滅させるべきか判定する
	/// 前方への接続がない、またはマップ外の場合に true を返す
	/// </summary>
	bool ShouldDestroyAtNode(
		No::Registry& registry,
		int nodeX,
		int nodeY,
		const No::Vector3& direction
	);

	/// <summary>
	/// 指定座標のグリッドセルを取得する
	/// </summary>
	GridCellComponent* GetGridCell(
		No::Registry& registry,
		int x,
		int y
	);
};