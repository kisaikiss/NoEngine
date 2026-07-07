#include "stdafx.h"
#include "StaminaUISystem.h"
#include "application/ClockworksDisease/Component/UI/UserInterfaceComponent.h"
#include "application/ClockworksDisease/Component/Player/PlayerComponent.h"

void StaminaUISystem::Update(No::Registry& registry, float deltaTime) {

	float stamina = 0.0f;
	float maxStamina = 0.0f;
	for (auto e : registry.View<PlayerComponent>()) {
		auto* player = registry.GetComponent<PlayerComponent>(e);
		stamina = player->stamina;
		maxStamina = player->maxStamina;
	}

	if (maxStamina == 0.0f) {
		return;
	}

	for (auto e : registry.View<No::SpriteComponent, StaminaGaugeComponent>()) {
		auto* sprite = registry.GetComponent<No::SpriteComponent>(e);
		auto* staminaGauge = registry.GetComponent<StaminaGaugeComponent>(e);

		// 前フレームと同じスタミナかつスタミナが最大値ならタイマーを進める
		if (staminaGauge->preStamina == stamina && stamina == maxStamina) {
			staminaGauge->timer += deltaTime;
		} else { // 違うなら表示する
			sprite->color.a = 1.0f;
			staminaGauge->alphaT = 0.0f;
			staminaGauge->timer = 0.0f;
		}

		// もしタイマーが一定以上なら少しずつ透明にする
		if (staminaGauge->timer >= staminaGauge->disappearanceTime) {
			if (staminaGauge->alphaT < 1.0f) {
				staminaGauge->alphaT += deltaTime;
				sprite->color.a = std::lerp(1.0f, 0.0f, staminaGauge->alphaT);
			}
		}

		// ゲージ最小値 (角度の数値が多いほど描画しない範囲が増えていくため)
		constexpr float kGaugeMin = 2 * PI;
		// スタミナの数値から描画しない範囲を決める
		sprite->nonRenderAngle = kGaugeMin - (stamina * kGaugeMin / maxStamina);

		// 次のフレームのためにこのフレームのスタミナを入れておく
		staminaGauge->preStamina = stamina;
	}
}
