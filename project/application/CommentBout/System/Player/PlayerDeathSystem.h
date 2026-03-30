#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// プレイヤー死亡カットシーンを管理するシステム。
/// PlayerAnimState::Dead を検知したら:
///   1. inCutscene=true / isPause=true でゲームをポーズ
///   2. プレイヤースプライトを毎フレーム Y+ 方向に落下移動
///   3. 画面外に出たら ClearOverStateComponent を Over で起動
/// SetStopInPause(false) によりポーズ中も動作する。
/// </summary>
class PlayerDeathSystem : public No::ISystem {
public:
	PlayerDeathSystem() { SetStopInPause(false); }
	void Update(No::Registry& registry, float deltaTime) override;
private:
	bool cutsceneStarted_ = false;
};
