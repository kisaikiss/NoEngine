#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// ボスの行動を管理するシステムクラス
/// </summary>
class BossBehaviorSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
};
