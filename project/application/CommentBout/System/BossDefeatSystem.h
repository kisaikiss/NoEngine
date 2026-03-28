#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// ボス撃破演出を管理するシステム。
/// BossDefeatSequenceComponent のフェーズを進め、爆発スプライト生成/消去、
/// ボスエンティティの破棄、その後の ClearOverStateComponent 起動を行う。
/// </summary>
class BossDefeatSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
};
