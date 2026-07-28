#include "stdafx.h"
#include "StaminaUISystem.h"
#include "application/ClockworksDisease/Component/UI/UserInterfaceComponent.h"
#include "application/ClockworksDisease/Component/Player/PlayerComponent.h"
#include "application/ClockworksDisease/Component/Player/PlayerMoveTags.h"

void StaminaUISystem::Update(No::Registry& registry, float deltaTime) {

	// プレイヤーの情報を収集
	float stamina = 0.0f;
	float maxStamina = 0.0f;
	bool canMagic = false;
	bool hasMagic = false;
	No::Vector3 playerPos = No::Vector3::ZERO;
	for (auto e : registry.View<PlayerComponent, No::TransformComponent> ()) {
		auto* player = registry.GetComponent<PlayerComponent>(e);
		stamina = player->stamina;
		maxStamina = player->maxStamina;
		canMagic = player->canCreateScaffold && registry.Has<CreateMagicScaffoldTag>(e);
		hasMagic = registry.Has<CreateMagicScaffoldTag>(e);
		playerPos = registry.GetComponent<No::TransformComponent>(e)->GetWorldPosition(registry);
	}
	// スタミナ最大値が0なら早期リターン
	if (maxStamina == 0.0f) {
		return;
	}

	// カメラの情報を収集
	No::Matrix4x4 view{};
	No::Matrix4x4 projection{};
	for (auto e : registry.View<No::CameraComponent, No::ActiveCameraTag>()) {
		auto* camera = registry.GetComponent<No::CameraComponent>(e);
		view = camera->view;
		projection = camera->projection;
	}


	// スタミナゲージの位置と減り具合を決める
	No::Color staminaGaugeColor = No::Color::WHITE;
	No::Vector2 staminaGaugePosition = No::Vector2::ZERO;
	for (auto e : registry.View<No::SpriteComponent, No::Transform2DComponent, StaminaGaugeComponent>()) {
		auto* sprite = registry.GetComponent<No::SpriteComponent>(e);
		auto* staminaGauge = registry.GetComponent<StaminaGaugeComponent>(e);

		// プレイヤーの位置にスタミナゲージを表示する
		auto* transform = registry.GetComponent<No::Transform2DComponent>(e);
		transform->translate = No::WorldToScreen(playerPos, view, projection);
		constexpr No::Vector2 uiPositionOffset = { -100.f,100.f };
		transform->translate += uiPositionOffset;


		// 前フレームと同じスタミナかつスタミナが最大値ならタイマーを進める
		if (staminaGauge->preStamina == stamina && stamina == maxStamina) {
			staminaGauge->timer += deltaTime;
		} else { // 違うなら表示する
			sprite->color.a = 1.0f;
			staminaGauge->alphaT = 0.0f;
			staminaGauge->timer = 0.0f;
		}

		if (hasMagic && !canMagic) {
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

		// スタミナゲージの装飾のためにスタミナゲージの情報を収集しておく
		staminaGaugeColor = sprite->color;
		staminaGaugePosition = transform->translate;

	}


	// スタミナゲージの周りの部分の色をスタミナゲージ本体に合わせる
	for (auto e : registry.View < No::Transform2DComponent, No::SpriteComponent, StaminaGaugeParentTag>()) {
		auto* sprite = registry.GetComponent<No::SpriteComponent>(e);
		sprite->color = staminaGaugeColor;

		// 魔法UI
		if (registry.Has<CanMagicUITag>(e)) {
			//プレイヤーが魔法を使えるレベルか
			if (!hasMagic) {
				sprite->isVisible = false;
			} else {
				// 魔法を使えるかつUIの背景なら常に見える
				if (registry.GetComponent<CanMagicUITag>(e)->isBackground) {
					sprite->isVisible = true;
				}
			}

		} 
	}
}
