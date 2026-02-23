#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// グリッド描画システム
/// グリッドの線を赤色で描画するシステム
///
/// GridToWorld は GridUtils::GridToWorld に移動した。
/// スケール変更は GridUtils.h の scale パラメータで行う。
/// </summary>
class GridRenderSystem : public No::ISystem {
public:
	/// <summary>
	/// 更新関数
	/// </summary>
	void Update(No::Registry& registry, float deltaTime) override;
};