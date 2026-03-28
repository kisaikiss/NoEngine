#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// クリア/オーバー画面の演出スプライトを生成・更新・破棄するシステム。
/// ClearOverStateComponent のフェーズに応じてフェード・ロゴ・メニューを制御する。
/// </summary>
class ClearOverViewSystem : public No::ISystem {
public:
	void Update(No::Registry& registry, float deltaTime) override;
 
};
