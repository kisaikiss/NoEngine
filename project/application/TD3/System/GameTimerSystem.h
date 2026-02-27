#pragma once
#include "engine/NoEngine.h"
#include "../Component/PlayerComponent.h"
#include "GameTimer.h"

/// <summary>
/// ゲームタイマー更新システム
/// PlayerComponentのisMovingフラグを見て、ゲームタイマーを更新する
/// 他のシステムより先に実行される必要がある
/// </summary>
class GameTimerSystem : public No::ISystem {
public:
	GameTimerSystem(GameTimer* timer, float* lastRealDeltaTime = nullptr) 
		: gameTimer_(timer), lastRealDeltaTime_(lastRealDeltaTime) {}

	void Update(No::Registry& registry, float deltaTime) override {
		if (!gameTimer_) return;

		// リアルdeltaTimeを保存（デバッグ表示用）
		if (lastRealDeltaTime_) {
			*lastRealDeltaTime_ = deltaTime;
		}

		// プレイヤーの移動状態を取得
		bool isPlayerMoving = false;
		auto playerView = registry.View<PlayerComponent, PlayerTag>();
		if (!playerView.Empty()) {
			auto it = playerView.begin();
			PlayerComponent* player = registry.GetComponent<PlayerComponent>(*it);
			if (player) {
				isPlayerMoving = player->isMoving;
			}
		}

		// ゲームタイマーを更新
		gameTimer_->Update(deltaTime, isPlayerMoving);
	}

private:
	GameTimer* gameTimer_;
	float* lastRealDeltaTime_;
};
