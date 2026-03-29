#pragma once
#include "engine/NoEngine.h"
#include "application/CommentBout/Component/OutGame/GameResultComponent.h"

/// <summary>
/// クリア/オーバー条件を監視して GameResultComponent.result を更新するシステム。
/// ・ボスが死亡 → Clear → BossDefeatSequenceComponent を起動
/// ・プレイヤーが死亡 / レール終端 → Over → ClearOverStateComponent を直接起動
/// デバッグ無効フラグは DebugShortcutStateComponent::debugDisableResult を参照。
/// </summary>
class GameResultSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
};
