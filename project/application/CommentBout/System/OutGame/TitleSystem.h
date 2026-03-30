#pragma once
#include "engine/NoEngine.h"

class TitleSystem : public No::ISystem {
public:
	TitleSystem() { SetStopInPause(false); }
	void Update(No::Registry& registry, float deltaTime) override;
private:
	// シーン遷移フェードイン中（約 0.75 秒）の誤入力を防ぐためのブロックタイマー。
	// SceneManager の transitionDuration(1.5f) / 2 に対応する。
	float entryBlockTimer_ = 0.75f;
};
