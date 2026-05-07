#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// InputHelper の内部状態を更新するシステム
/// 毎フレーム最初に実行し、InputHelper の内部状態（スティックリピート等）を更新するシステム。
/// </summary>
class InputHelperSystem : public No::ISystem {
public:
	InputHelperSystem() { SetStopInPause(false); }
	void Update(No::Registry& registry, float deltaTime) override;
};
