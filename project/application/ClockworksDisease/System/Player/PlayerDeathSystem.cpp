#include "stdafx.h"
#include "PlayerDeathSystem.h"
#include "application/ClockworksDisease/Component/Player/PlayerComponent.h"
#include "application/ClockworksDisease/Component/Player/PlayerDeathComponent.h"
#include "application/ClockworksDisease/Component/Camera/FollowCameraComponent.h"
#include "engine/Runtime/GraphicsCore.h"

namespace {

No::Entity CreateFadeOverlay(No::Registry& registry) {
	auto overlay = registry.GenerateEntity();
	No::Vector2 windowSize = NoEngine::GraphicsCore::GetWindowSize();

	auto* t2d = registry.AddComponent<No::Transform2DComponent>(overlay);
	t2d->scale = windowSize;
	t2d->translate = windowSize / 2.f;

	auto* sprite = registry.AddComponent<No::SpriteComponent>(overlay);
	sprite->space = NoEngine::SpriteSpace::Screen;
	sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/engine/white1x1.png");
	sprite->color = No::Color(0.f, 0.f, 0.f, 0.f);
	sprite->layer = 1000000; // 他の全UIより手前
	return overlay;
}

// ウィンドウサイズ変更にも追従させつつアルファを設定する
void UpdateOverlay(No::Registry& registry, No::Entity overlay, float alpha) {
	if (overlay == No::INVALID_ENTITY) return;
	No::Vector2 windowSize = NoEngine::GraphicsCore::GetWindowSize();
	if (auto* t2d = registry.GetComponent<No::Transform2DComponent>(overlay)) {
		t2d->scale = windowSize;
		t2d->translate = windowSize / 2.f;
	}
	if (auto* sprite = registry.GetComponent<No::SpriteComponent>(overlay)) {
		sprite->color.a = std::clamp(alpha, 0.0f, 1.0f);
	}
}

void SetPlayerVisible(No::Registry& registry, No::Entity player, bool visible) {
	if (auto* mesh = registry.GetComponent<No::MeshComponent>(player)) mesh->isVisible = visible;
	// isVisibleだけではレイトレの影が残るため、castShadowも合わせて切り替える(ItemGetSystemと同じ)
	if (auto* material = registry.GetComponent<No::MaterialComponent>(player)) material->castShadow = visible;
}

void EmitDeathEffect(No::Registry& registry, const No::Vector3& position) {
	for (auto e : registry.View<No::TransformComponent, No::EffectEmitterComponent, PlayerDeathEffectTag>()) {
		registry.GetComponent<No::TransformComponent>(e)->translate = position;
		registry.AddComponent<No::EffectEmitTag>(e);
	}
}

// 暗転中にカメラをリスポーン地点へ寄せる(元のPlayerDeathSystemの処理 + XZも即座にスナップ)
void SnapFollowCamera(No::Registry& registry, const No::Vector3& playerPos) {
	for (auto e : registry.View<FollowCameraComponent, No::TransformComponent>()) {
		auto* camera = registry.GetComponent<FollowCameraComponent>(e);
		auto* transform = registry.GetComponent<No::TransformComponent>(e);
		camera->phi = 1.0f;
		camera->currentDistance = 0.1f;
		transform->translate.x = playerPos.x + camera->currentDistance * std::sin(camera->phi) * std::cos(camera->theta);
		transform->translate.y = playerPos.y + camera->currentDistance * std::cos(camera->phi);
		transform->translate.z = playerPos.z + camera->currentDistance * std::sin(camera->phi) * std::sin(camera->theta);
	}
}

float Progress(float timer, float duration) {
	return duration > 0.0f ? std::clamp(timer / duration, 0.0f, 1.0f) : 1.0f;
}

} // namespace

void PlayerDeathSystem::Update(No::Registry& registry, float deltaTime) {
	using Phase = PlayerDeathSequenceComponent::Phase;

	for (auto e : registry.View<PlayerComponent, No::TransformComponent, No::VelocityComponent>()) {
		auto* player = registry.GetComponent<PlayerComponent>(e);
		auto* transform = registry.GetComponent<No::TransformComponent>(e);
		auto* velocity = registry.GetComponent<No::VelocityComponent>(e);
		auto* seq = registry.GetComponent<PlayerDeathSequenceComponent>(e);

		// --- 演出開始 ---
		if (!seq) {
			if (transform->GetWorldPosition(registry).y >= player->deathHeight) continue;

			seq = registry.AddComponent<PlayerDeathSequenceComponent>(e);
			seq->deathPosition = transform->GetWorldPosition(registry);
			SetPlayerVisible(registry, e, false);
			EmitDeathEffect(registry, seq->deathPosition);
		}

		// リスポーン位置へ移動するまでは速度を止める。kBlackOut以降は操作を許可する。
		if (seq->phase != Phase::kBlackOut && seq->phase != Phase::kFadeIn) {
			velocity->linear = No::Vector3::ZERO;
			player->yVelocity = 0.0f;
		}
		seq->timer += deltaTime;

		switch (seq->phase) {
		case Phase::kVanish: {
			// 非表示のまま死亡位置に固定しておく
			transform->SetWorldPosition(registry, seq->deathPosition);
			if (seq->timer >= seq->vanishDuration) {
				seq->overlayEntity = CreateFadeOverlay(registry);
				seq->timer = 0.0f;
				seq->phase = Phase::kFadeOut;
			}
			break;
		}
		case Phase::kFadeOut: {
			transform->SetWorldPosition(registry, seq->deathPosition);
			float t = Progress(seq->timer, seq->fadeOutDuration);
			UpdateOverlay(registry, seq->overlayEntity, t);
			if (t >= 1.0f) {
				// 完全に暗転したのでここでリスポーンさせる
				transform->SetWorldPosition(registry, player->respawnPoint);
				SetPlayerVisible(registry, e, true);
				SnapFollowCamera(registry, transform->GetWorldPosition(registry));
				seq->timer = 0.0f;
				seq->phase = Phase::kBlackOut;
			}
			break;
		}
		case Phase::kBlackOut: {
			UpdateOverlay(registry, seq->overlayEntity, 1.0f);
			if (seq->timer >= seq->blackOutDuration) {
				seq->timer = 0.0f;
				seq->phase = Phase::kFadeIn;
			}
			break;
		}
		case Phase::kFadeIn: {
			float t = Progress(seq->timer, seq->fadeInDuration);
			UpdateOverlay(registry, seq->overlayEntity, 1.0f - t);
			if (t >= 1.0f) {
				registry.DestroyEntity(seq->overlayEntity);
				registry.RemoveComponent<PlayerDeathSequenceComponent>(e); // ロック解除
			}
			break;
		}
		}
	}
}

REGISTER_SYSTEM(::PlayerDeathSystem, "PlayerDeathSystem", "Gameplay")
