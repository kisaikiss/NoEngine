#include "stdafx.h"
#include "CameraIntroSystem.h"
#include "../../Component/Camera/CameraIntroComponent.h"
#include "engine/Runtime/GraphicsCore.h"

void CameraIntroSystem::Update(No::Registry& registry, float deltaTime) {
	// 注意: Viewの絞り込み条件にCameraIntroLockTagは含めない。
	// kFadeOut完了時にRemoveComponent<CameraIntroLockTag>するため、
	// 含めてしまうと次フレーム以降このEntityがViewから外れ、
	// kFadeIn(暗転復帰)が二度と処理されなくなる。
	for (auto e : registry.View<CameraIntroComponent>()) {
		auto* intro = registry.GetComponent<CameraIntroComponent>(e);

		switch (intro->phase) {
		case CameraIntroComponent::Phase::kPlayingRoutine: {
			auto* routine = registry.GetComponent<No::TransformRoutineComponent>(e);
			// loop=falseのTransformRoutineが最後まで再生し終えるとplayingがfalseになる
			if (!routine || !routine->playing) {
				intro->overlayEntity = CreateFadeOverlay(registry);
				intro->fadeTimer = 0.0f;
				intro->phase = CameraIntroComponent::Phase::kFadeOut;
			}
			break;
		}
		case CameraIntroComponent::Phase::kFadeOut: {
			intro->fadeTimer += deltaTime;
			float t = intro->fadeOutDuration > 0.0f
				? std::clamp(intro->fadeTimer / intro->fadeOutDuration, 0.0f, 1.0f)
				: 1.0f;
			SetOverlayAlpha(registry, intro->overlayEntity, t);

			if (t >= 1.0f) {
				// 画面が完全に暗転したタイミングでプレイヤー追従へ切り替える
				registry.RemoveComponent<CameraIntroLockTag>(e);
				intro->fadeTimer = 0.0f;
				intro->phase = CameraIntroComponent::Phase::kFadeIn;
			}
			break;
		}
		case CameraIntroComponent::Phase::kFadeIn: {
			intro->fadeTimer += deltaTime;
			float t = intro->fadeInDuration > 0.0f
				? std::clamp(intro->fadeTimer / intro->fadeInDuration, 0.0f, 1.0f)
				: 1.0f;
			SetOverlayAlpha(registry, intro->overlayEntity, 1.0f - t);

			if (t >= 1.0f) {
				registry.DestroyEntity(intro->overlayEntity);
				intro->overlayEntity = No::INVALID_ENTITY;
				intro->phase = CameraIntroComponent::Phase::kDone;
			}
			break;
		}
		case CameraIntroComponent::Phase::kDone:
		default:
			break;
		}
	}
}

No::Entity CameraIntroSystem::CreateFadeOverlay(No::Registry& registry) {
	auto overlay = registry.GenerateEntity();

	No::Vector2 windowSize = NoEngine::GraphicsCore::GetWindowSize();

	auto* t2d = registry.AddComponent<No::Transform2DComponent>(overlay);
	t2d->scale = windowSize;
	t2d->translate = windowSize / 2.f;

	auto* sprite = registry.AddComponent<No::SpriteComponent>(overlay);
	sprite->space = NoEngine::SpriteSpace::Screen;
	sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/engine/white1x1.png");
	sprite->color = No::Color(0.f, 0.f, 0.f, 0.f);
	sprite->layer = 1000000; // 他の全UIより手前に描画する

	return overlay;
}

void CameraIntroSystem::SetOverlayAlpha(No::Registry& registry, No::Entity overlay, float alpha) {
	if (overlay == No::INVALID_ENTITY) return;
	if (auto* sprite = registry.GetComponent<No::SpriteComponent>(overlay)) {
		sprite->color.a = std::clamp(alpha, 0.0f, 1.0f);
	}
}