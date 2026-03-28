#include "stdafx.h"
#include "PlayerInfoDebugSystem.h"
#include "application/CommentBout/Component/PlayerComponent.h"
#include "application/CommentBout/Component/PlayerAttackComponent.h"
#include "application/CommentBout/Component/HealthComponent.h"
#include "application/CommentBout/Component/InvincibleComponent.h"
#include "application/CommentBout/Component/PlayerHitboxComponent.h"
#include "application/CommentBout/Data/PlayerConfig.h"
#include "application/CommentBout/Data/PlayerDataIO.h"
#include "application/CommentBout/Component/DamageFlashComponent.h"
#include "application/CommentBout/GameTag.h"
#include <algorithm>

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

namespace {
PlayerHitboxComponent* FindPlayerHitbox(No::Registry& registry, No::Entity playerEntity) {
	for (auto hitboxEntity : registry.View<CBPlayerHitboxTag, PlayerHitboxComponent>()) {
		auto* hitbox = registry.GetComponent<PlayerHitboxComponent>(hitboxEntity);
		if (hitbox && hitbox->playerEntity == playerEntity) {
			return hitbox;
		}
	}
	return nullptr;
}

void ApplyConfigToRuntime(
	No::Registry& registry,
	No::Entity playerEntity,
	PlayerConfig& config,
	PlayerComponent& player,
	PlayerAttackComponent& attack,
	HealthComponent& health,
	InvincibleComponent& invincible,
	No::Transform2DComponent& transform,
	No::StartTransform2DComponent* startTransform,
	PlayerHitboxComponent* hitbox
) {
	player.moveSpeed = config.moveSpeed;
	player.acceleration = config.acceleration;
	player.deceleration = config.deceleration;
	player.maxSpeed = config.maxSpeed;
	player.invincibleDurationDefault = config.invincibleDurationDefault;

	attack.spawnOffset = config.attackSpawnOffset;
	attack.attackSize = config.attackSize;
	attack.visibleTime = config.attackVisibleTime;
	attack.attackLayer = config.attackLayer;
	attack.attackPower = config.attackPower;

	health.maxHp = std::max(1, config.playerMaxHp);
	health.hp = std::min(std::max(1, health.hp), health.maxHp);
	invincible.duration = std::max(0.01f, config.invincibleDuration);

	if (startTransform) {
		startTransform->translate = config.startPosition;
		startTransform->scale = config.startScale;
		startTransform->rotation = config.startRotation;
		transform.translate = startTransform->translate;
		transform.scale = startTransform->scale;
		transform.rotation = startTransform->rotation;
	}

	if (hitbox) {
		hitbox->useCameraGateForPlayerHit = config.useCameraGateForPlayerHit;
		hitbox->cameraGateNear = config.cameraGateNear;
		hitbox->cameraGateDepth = config.cameraGateDepth;
		hitbox->cameraGateHalfWidth = config.cameraGateHalfWidth;
		hitbox->cameraGateHalfHeight = config.cameraGateHalfHeight;
	}

	auto* flash = registry.GetComponent<DamageFlashComponent>(playerEntity);
	if (flash) {
		flash->duration = std::max(0.01f, config.flashDuration);
		flash->blinkEnabled = config.flashBlinkEnabled;
		flash->blinkHz = std::max(0.0f, config.flashBlinkHz);
		flash->minAlpha = std::max(0.0f, std::min(1.0f, config.flashMinAlpha));
		flash->maxAlpha = std::max(flash->minAlpha, std::min(1.0f, config.flashMaxAlpha));
		flash->flashColor = No::Color(config.flashColorRGB.x, config.flashColorRGB.y, config.flashColorRGB.z, flash->maxAlpha);
	}
}

void CaptureRuntimeToConfig(
	const PlayerComponent& player,
	const PlayerAttackComponent& attack,
	const HealthComponent& health,
	const InvincibleComponent& invincible,
	const No::StartTransform2DComponent* startTransform,
	const PlayerHitboxComponent* hitbox,
	PlayerConfig& config
) {
	config.moveSpeed = player.moveSpeed;
	config.acceleration = player.acceleration;
	config.deceleration = player.deceleration;
	config.maxSpeed = player.maxSpeed;
	config.invincibleDurationDefault = player.invincibleDurationDefault;

	config.attackSpawnOffset = attack.spawnOffset;
	config.attackSize = attack.attackSize;
	config.attackVisibleTime = attack.visibleTime;
	config.attackLayer = attack.attackLayer;
	config.attackPower = attack.attackPower;

	config.playerMaxHp = health.maxHp;
	config.invincibleDuration = invincible.duration;

	if (startTransform) {
		config.startPosition = startTransform->translate;
		config.startScale = startTransform->scale;
		config.startRotation = startTransform->rotation;
	}
	if (hitbox) {
		config.useCameraGateForPlayerHit = hitbox->useCameraGateForPlayerHit;
		config.cameraGateNear = hitbox->cameraGateNear;
		config.cameraGateDepth = hitbox->cameraGateDepth;
		config.cameraGateHalfWidth = hitbox->cameraGateHalfWidth;
		config.cameraGateHalfHeight = hitbox->cameraGateHalfHeight;
	}
}
}

void PlayerInfoDebugSystem::Update(No::Registry& registry, float deltaTime)
{
	static_cast<void>(deltaTime);

#ifdef USE_IMGUI
	ImGui::Begin("自機設定");

	PlayerConfig* playerConfig = nullptr;
	for (auto configEntity : registry.View<CBPlayerConfigTag, PlayerConfig>()) {
		playerConfig = registry.GetComponent<PlayerConfig>(configEntity);
		if (playerConfig) {
			break;
		}
	}

	auto view = registry.View<CBPlayerTag, PlayerComponent, PlayerAttackComponent, HealthComponent, InvincibleComponent, No::Transform2DComponent>();
	bool hasPlayer = false;
	for (auto entity : view) {
		hasPlayer = true;
		auto* player = registry.GetComponent<PlayerComponent>(entity);
		auto* attack = registry.GetComponent<PlayerAttackComponent>(entity);
		auto* health = registry.GetComponent<HealthComponent>(entity);
		auto* invincible = registry.GetComponent<InvincibleComponent>(entity);
		auto* transform = registry.GetComponent<No::Transform2DComponent>(entity);
		auto* startTransform = registry.GetComponent<No::StartTransform2DComponent>(entity);
		auto* hitbox = FindPlayerHitbox(registry, entity);
		if (!player || !attack || !health || !invincible || !transform || !playerConfig) {
			continue;
		}

		if (ImGui::Button("JSON読込（即時反映）")) {
			*playerConfig = PlayerDataIO::Load();
			ApplyConfigToRuntime(registry, entity, *playerConfig, *player, *attack, *health, *invincible, *transform, startTransform, hitbox);
		}
		ImGui::SameLine();
		if (ImGui::Button("JSON保存")) {
			CaptureRuntimeToConfig(*player, *attack, *health, *invincible, startTransform, hitbox, *playerConfig);
			PlayerDataIO::Save(*playerConfig);
		}
		ImGui::Separator();

		if (ImGui::CollapsingHeader("移動設定", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::DragFloat("移動速度", &player->moveSpeed, 1.0f, 1.0f, 3000.0f);
			ImGui::DragFloat("加速度", &player->acceleration, 10.0f, 0.0f, 10000.0f);
			ImGui::DragFloat("減速度", &player->deceleration, 10.0f, 0.0f, 10000.0f);
			ImGui::DragFloat("最高速度", &player->maxSpeed, 1.0f, 1.0f, 3000.0f);
			ImGui::Text("現在速度: (%.1f, %.1f)", player->velocity.x, player->velocity.y);
			ImGui::Text("現在位置: (%.1f, %.1f)", transform->translate.x, transform->translate.y);
		}

		if (ImGui::CollapsingHeader("無敵設定", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::DragFloat("無敵時間(標準)", &player->invincibleDurationDefault, 0.01f, 0.01f, 10.0f);
			ImGui::DragFloat("無敵時間(現在運用)", &invincible->duration, 0.01f, 0.01f, 10.0f);
			ImGui::Checkbox("デバッグ無敵", &player->debugInvincible);
			ImGui::Text("無敵タイマー残り: %.2f", invincible->time);
		}

		if (ImGui::CollapsingHeader("被弾点滅設定", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::DragFloat("点滅時間", &playerConfig->flashDuration, 0.01f, 0.01f, 2.0f);
			ImGui::Checkbox("点滅有効", &playerConfig->flashBlinkEnabled);
			ImGui::DragFloat("点滅周波数(Hz)", &playerConfig->flashBlinkHz, 0.1f, 0.0f, 60.0f);
			ImGui::DragFloat("最小アルファ", &playerConfig->flashMinAlpha, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("最大アルファ", &playerConfig->flashMaxAlpha, 0.01f, 0.0f, 1.0f);
			ImGui::ColorEdit3("点滅色(RGB)", &playerConfig->flashColorRGB.x);

			auto* flash = registry.GetComponent<DamageFlashComponent>(entity);
			if (flash) {
				flash->duration = std::max(0.01f, playerConfig->flashDuration);
				flash->blinkEnabled = playerConfig->flashBlinkEnabled;
				flash->blinkHz = std::max(0.0f, playerConfig->flashBlinkHz);
				flash->minAlpha = std::max(0.0f, std::min(1.0f, playerConfig->flashMinAlpha));
				flash->maxAlpha = std::max(flash->minAlpha, std::min(1.0f, playerConfig->flashMaxAlpha));
				flash->flashColor = No::Color(playerConfig->flashColorRGB.x, playerConfig->flashColorRGB.y, playerConfig->flashColorRGB.z, flash->maxAlpha);
			}
		}

		if (ImGui::CollapsingHeader("攻撃設定", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::DragFloat2("攻撃生成オフセット", &attack->spawnOffset.x, 1.0f, -1000.0f, 1000.0f);
			ImGui::DragFloat2("攻撃サイズ", &attack->attackSize.x, 1.0f, 1.0f, 2000.0f);
			ImGui::DragFloat("攻撃表示時間", &attack->visibleTime, 0.01f, 0.01f, 5.0f);
			ImGui::DragInt("攻撃レイヤー", &attack->attackLayer, 1.0f, 0, 2000);
			ImGui::DragInt("攻撃力", &attack->attackPower, 1.0f, 1, 999);
		}

		if (ImGui::CollapsingHeader("耐久設定", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::DragInt("最大HP", &health->maxHp, 1.0f, 1, 999);
			health->maxHp = std::max(1, health->maxHp);
			health->hp = std::min(health->hp, health->maxHp);
			ImGui::Text("現在HP: %d", health->hp);
		}

		if (ImGui::CollapsingHeader("当たり判定設定（カメラゲート）", ImGuiTreeNodeFlags_DefaultOpen) && hitbox) {
			ImGui::Checkbox("カメラゲート判定を使用", &hitbox->useCameraGateForPlayerHit);
			ImGui::DragFloat("Near", &hitbox->cameraGateNear, 0.01f, 0.0f, 10.0f);
			ImGui::DragFloat("Depth", &hitbox->cameraGateDepth, 0.01f, 0.01f, 10.0f);
			ImGui::DragFloat("Half Width", &hitbox->cameraGateHalfWidth, 0.01f, 0.01f, 10.0f);
			ImGui::DragFloat("Half Height", &hitbox->cameraGateHalfHeight, 0.01f, 0.01f, 10.0f);
		}

		if (ImGui::CollapsingHeader("初期配置設定", ImGuiTreeNodeFlags_DefaultOpen) && startTransform) {
			ImGui::DragFloat2("開始位置", &startTransform->translate.x, 1.0f, -5000.0f, 5000.0f);
			ImGui::DragFloat2("開始スケール", &startTransform->scale.x, 1.0f, 1.0f, 5000.0f);
			ImGui::DragFloat("開始回転", &startTransform->rotation, 0.01f, -6.28318f, 6.28318f);
			if (ImGui::Button("開始配置を現在位置へ反映")) {
				transform->translate = startTransform->translate;
				transform->scale = startTransform->scale;
				transform->rotation = startTransform->rotation;
			}
		}

		if (ImGui::CollapsingHeader("コライダー設定（自機 2D）", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::DragFloat2("ローカルオフセット##player2D", &playerConfig->playerCollider2D.localOffset2D.x, 0.5f, -500.0f, 500.0f);
			ImGui::DragFloat2("サイズ倍率##player2D", &playerConfig->playerCollider2D.sizeMultiplier2D.x, 0.01f, 0.01f, 10.0f);
		}

		if (ImGui::CollapsingHeader("コライダー設定（攻撃 2D）", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::DragFloat2("ローカルオフセット##attack2D", &playerConfig->attackCollider.localOffset2D.x, 0.5f, -500.0f, 500.0f);
			ImGui::DragFloat2("サイズ倍率##attack2D", &playerConfig->attackCollider.sizeMultiplier2D.x, 0.01f, 0.01f, 10.0f);
		}

		if (ImGui::Button("コライダー設定を JSON 保存")) {
			CaptureRuntimeToConfig(*player, *attack, *health, *invincible, startTransform, hitbox, *playerConfig);
			PlayerDataIO::Save(*playerConfig);
		}

		CaptureRuntimeToConfig(*player, *attack, *health, *invincible, startTransform, hitbox, *playerConfig);

		ImGui::Separator();
		ImGui::Text("JSON読込後は即座に自機へ反映されます。");
		break;
	}

	if (!hasPlayer) {
		ImGui::Text("自機が見つかりません。");
	}
	ImGui::End();
#else
	static_cast<void>(registry);
#endif
}
