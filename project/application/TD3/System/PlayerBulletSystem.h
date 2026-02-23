#pragma once
#include "engine/NoEngine.h"
#include "../Component/PlayerBulletComponent.h"
#include "../Component/GridCellComponent.h"

/// <summary>
/// プレイヤー弾丸システム
/// 弾丸の移動・グリッドベース消滅・最大距離消滅を管理する
/// </summary>
class PlayerBulletSystem : public No::ISystem {
public:
	/// <summary>
	/// 更新処理
	/// </summary>
	void Update(No::Registry& registry, float deltaTime) override;

private:
	/// <summary>
	/// 指定座標のグリッドセルを取得する
	/// 見つからない場合は nullptr を返す
	/// </summary>
	GridCellComponent* GetGridCell(No::Registry& registry, int x, int y);

	/// <summary>
	/// 弾の進行方向ベクトルからグリッド接続の有無を判定する
	/// ノード上で前方に接続がなければ true（消滅すべき）を返す
	/// </summary>
	bool ShouldDestroyAtNode(
		No::Registry& registry,
		int nodeX, int nodeY,
		const No::Vector3& direction
	);
};