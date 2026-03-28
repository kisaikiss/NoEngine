#pragma once
#include "engine/NoEngine.h"
#include "application/CommentBout/Data/EnemyConfig.h"

/// <summary>
/// 敵プリセット設定の ImGui エディタシステム。
/// 値変更時に生存中の敵へ即時反映する。
/// </summary>
class EnemyConfigEditorSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;

private:
	EnemyConfigMap presets_;
	bool loaded_ = false;

	void ApplyToAliveEnemies(No::Registry& registry);
};
