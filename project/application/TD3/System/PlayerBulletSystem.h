#pragma once
#include "engine/NoEngine.h"
#include "../Component/PlayerBulletComponent.h"
#include "../Component/GridCellComponent.h"

/// <summary>
/// プレイヤー弾丸システム
/// 弾丸の移動・消滅判定を管理する。
///
/// GetGridCell は GridUtils に移動した。
/// </summary>
class PlayerBulletSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;

private:
	/// <summary>
	/// 指定ノードで弾丸を消滅させるべきか判定する。
	/// 前方への接続がない、またはマップ外の場合に true を返す。
	/// </summary>
	bool ShouldDestroyAtNode(
		No::Registry& registry,
		int nodeX,
		int nodeY,
		const No::Vector3& direction
	);
};