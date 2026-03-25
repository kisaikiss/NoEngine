#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// プレイヤー関連の状態をImGuiで可視化するデバッグ用System
/// </summary>
class PlayerInfoDebugSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
};
