#pragma once
#include "engine/NoEngine.h"

// 毎フレーム最初に実行し、InputHelper の内部状態（スティックリピート等）を更新するシステム。
// GameScene・TitleScene の先頭に登録すること。
class InputHelperSystem : public No::ISystem {
public:
	InputHelperSystem() { SetStopInPause(false); }
	void Update(No::Registry& registry, float deltaTime) override;
};
