#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// クリア/オーバー画面のフェーズ管理・入力処理・シーン遷移を行うシステム。
/// PauseSystem の構造を踏襲する。
/// </summary>
class ClearOverSystem : public No::ISystem {
public:
	ClearOverSystem() { SetStopInPause(false); }
	void Update(No::Registry& registry, float deltaTime) override;
};
