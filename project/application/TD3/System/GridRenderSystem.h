#pragma once
#include "engine/NoEngine.h"


/// <summary>
/// グリッド描画システム
/// グリッドの線を赤色で描画するシステム
/// </summary>
class GridRenderSystem : public No::ISystem {
public:
	/// <summary>
	/// 更新関数
	/// </summary>
	void Update(No::Registry& registry, float deltaTime) override;

private:
	/// <summary>
	/// グリッド座標を3Dワールド座標に変換
	/// </summary>
	/// <param name="x">グリッドのX座標</param>
	/// <param name="y">グリッドのY座標</param>
	/// <returns>3Dワールド座標を表すVector3</returns>
	No::Vector3 GridToWorld(int x, int y);
};
