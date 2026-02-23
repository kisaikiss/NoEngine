#pragma once
#include "engine/NoEngine.h"
#include "../GameTag.h"

/// <summary>
/// デスシステム
/// DeathFlag が true のエンティティを一括削除する
/// 全システムの最後に実行されれば安全な削除(これが最後に動いているかどうかがわからない)
/// </summary>
class DeathSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
};
