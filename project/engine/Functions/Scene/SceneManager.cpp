#include "SceneManager.h"

#include "engine/Runtime/GraphicsCore.h"
#include "engine/Assets/Texture/TextureManager.h"
#include "engine/Functions/ECS/Component/SpriteComponent.h"
#include "engine/Functions/ECS/Component/Transform2DComponent.h"
#include "engine/Math/Easing.h"

#include <algorithm>

namespace NoEngine {
namespace Scene {
void SceneManager::CreateCircleOverlay(float initialAlpha, float initialScale) {
	if (!currentScene_) return;

	auto registry = currentScene_->GetRegistry();
	if (!registry) return;

	auto winSize = GraphicsCore::gWindowManager.GetMainWindow()->GetWindowSize();
	float winW = static_cast<float>(winSize.clientWidth);
	float winH = static_cast<float>(winSize.clientHeight);

	float maxSide = std::max(winW, winH);
	float coverSize = maxSide * 3.f;

	overlayEntity_ = registry->GenerateEntity();

	auto* t2d = registry->AddComponent<Component::Transform2DComponent>(overlayEntity_);
	t2d->scale = { coverSize * initialScale, coverSize * initialScale };
	t2d->translate = { winW * 0.5f, winH * 0.5f };

	auto* sprite = registry->AddComponent<Component::SpriteComponent>(overlayEntity_);
	sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Sprite/circle_soft.png");
	sprite->name = "SceneCircleScaleOverlay";
	sprite->layer = 100000;
	sprite->color = { 0.f, 0.f, 0.f, std::clamp(initialAlpha, 0.0f, 1.0f) };
}

void SceneManager::UpdateOverlay(float alpha, float scale) {
	if (!currentScene_) return;

	auto registry = currentScene_->GetRegistry();
	if (!registry) return;

	if (overlayEntity_ == 0) return;
	if (!registry->Has<Component::SpriteComponent>(overlayEntity_)) return;
	if (!registry->Has<Component::Transform2DComponent>(overlayEntity_)) return;

	auto winSize = GraphicsCore::gWindowManager.GetMainWindow()->GetWindowSize();
	float winW = static_cast<float>(winSize.clientWidth);
	float winH = static_cast<float>(winSize.clientHeight);

	float maxSide = std::max(winW, winH);
	float coverSize = maxSide * 3.0f;

	auto* sp = registry->GetComponent<Component::SpriteComponent>(overlayEntity_);
	auto* t2d = registry->GetComponent<Component::Transform2DComponent>(overlayEntity_);

	sp->color.a = std::clamp(alpha, 0.0f, 1.0f);

	float s = std::max(scale, 0.0f);
	t2d->scale = { coverSize * s, coverSize * s };
	t2d->translate = { winW * 0.5f, winH * 0.5f };
}

void SceneManager::DestroyOverlay() {
	if (!currentScene_) return;

	auto registry = currentScene_->GetRegistry();
	if (!registry) return;

	if (overlayEntity_ != 0) {
		if (registry->Has<Component::SpriteComponent>(overlayEntity_)) {
			registry->DestroyEntity(overlayEntity_);
		}
	}
	overlayEntity_ = 0;
}

void SceneManager::ChangeScene(const std::string& name, bool immediate) {
	auto it = factories_.find(name);
	if (it == factories_.end()) return;

	// 連続呼び出し防止
	if (isChanging_) return;

	isChanging_ = true;

	// 即時切替（従来通り）
	if (immediate || !currentScene_) {
		if (currentScene_) {
			currentScene_->OnExit();
		}

		currentScene_ = it->second();
		currentScene_->SetName(name);
		currentScene_->Setup();
		currentScene_->OnEnter();

		// 状態クリア
		isTransitioning_ = false;
		transitionPhase_ = TransitionPhase::None;
		transitionTimer_ = 0.0f;
		overlayEntity_ = 0;

		isChanging_ = false;
		return;
	}

	//遷移開始
	pendingName_ = name;
	isTransitioning_ = true;
	transitionPhase_ = TransitionPhase::FadingOut;
	transitionTimer_ = 0.0f;

	// 最初は小さい円
	CreateCircleOverlay(0.0f, 0.0f);
}

void SceneManager::Update(ComputeContext& ctx, float deltaTime) {
	// 遷移中
	if (isTransitioning_) {
		float half = transitionDuration_ * 0.5f;

		// deltaTime 異常値対策
		if (deltaTime > 0.0f && deltaTime < 0.1f) {
			transitionTimer_ += deltaTime;
		}

		if (transitionPhase_ == TransitionPhase::FadingOut) {
			float t = std::clamp(transitionTimer_ / half, 0.0f, 1.0f);
			float eased = NoEngine::Easing::EaseInOutSine(0.0f, 1.0f, t);

			// scale 0 -> 1
			// alpha 0 -> 1
			UpdateOverlay(eased, eased);

			if (transitionTimer_ >= half) {
				// シーン切替
				if (currentScene_) currentScene_->OnExit();

				auto it = factories_.find(pendingName_);
				if (it != factories_.end()) {
					currentScene_ = it->second();
					currentScene_->SetName(pendingName_);
					currentScene_->Setup();
					currentScene_->OnEnter();
				}

				// 新シーン側で overlay 作り直し
				overlayEntity_ = 0;
				CreateCircleOverlay(1.0f, 1.0f);

				// 次へ
				transitionPhase_ = TransitionPhase::FadingIn;
				transitionTimer_ = 0.0f;
			}
		} else if (transitionPhase_ == TransitionPhase::FadingIn) {
			float t = std::clamp(transitionTimer_ / half, 0.0f, 1.0f);
			float eased = NoEngine::Easing::EaseInOutSine(0.0f, 1.0f, t);

			float s = 1.0f - eased;
			UpdateOverlay(s, s);

			if (transitionTimer_ >= half) {
				DestroyOverlay();

				isTransitioning_ = false;
				transitionPhase_ = TransitionPhase::None;
				transitionTimer_ = 0.0f;

				isChanging_ = false;
			}
		}

		// 遷移中でも Scene Update
		if (currentScene_) {
			currentScene_->Update(ctx, deltaTime);
		}
		return;
	}

	// 通常更新
	if (currentScene_) currentScene_->Update(ctx, deltaTime);
}

}
}
