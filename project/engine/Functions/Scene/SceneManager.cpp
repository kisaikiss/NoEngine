#include "SceneManager.h"

#include "CircleScaleTransitionEffect.h"
#include "FadeTransitionEffect.h"

namespace NoEngine {
namespace Scene {
SceneManager::SceneManager() {
	RegisterTransitionEffect("CircleScale", [] { return std::make_unique<CircleScaleTransitionEffect>(); });
	RegisterTransitionEffect("Fade", [] { return std::make_unique<FadeTransitionEffect>(); });
}

std::unique_ptr<ITransitionEffect> SceneManager::CreateTransitionEffect(
	Event::SceneTransitionType type, const std::string& customName) {
	std::string key;
	switch (type) {
	case Event::SceneTransitionType::kCircleScale: key = "CircleScale"; break;
	case Event::SceneTransitionType::kFade:        key = "Fade"; break;
	case Event::SceneTransitionType::kCustom:      key = customName; break;
	default: break;
	}
	auto it = transitionFactories_.find(key);
	if (it == transitionFactories_.end()) {
		LogWarning("SceneManager: transition '" + key + "' not found. fallback to CircleScale.");
		it = transitionFactories_.find("CircleScale");
	}
	return it->second();
}

void SceneManager::ChangeScene(const std::string& name, bool immediate,
	Event::SceneTransitionType transitionType, const std::string& customTransitionName) {
	auto it = factories_.find(name);
	if (it == factories_.end()) return;
	if (isChanging_) return;
	isChanging_ = true;

	if (immediate || !currentScene_) {
		if (currentScene_) currentScene_->OnExit();
		currentScene_ = it->second();
		currentScene_->SetName(name);
		currentScene_->Setup();
		currentScene_->OnEnter();

		isTransitioning_ = false;
		transitionPhase_ = TransitionPhase::None;
		transitionTimer_ = 0.0f;
		currentEffect_.reset();
		isChanging_ = false;
		return;
	}

	pendingName_ = name;
	isTransitioning_ = true;
	transitionPhase_ = TransitionPhase::FadingOut;
	transitionTimer_ = 0.0f;

	currentEffect_ = CreateTransitionEffect(transitionType, customTransitionName);
	if (currentEffect_ && currentScene_->GetRegistry()) {
		currentEffect_->OnFadingOutStart(*currentScene_->GetRegistry());
	}
}

void SceneManager::Update(ComputeContext& ctx, float deltaTime) {
	if (isTransitioning_) {
		float half = transitionDuration_ * 0.5f;
		if (deltaTime > 0.0f && deltaTime < 0.1f) transitionTimer_ += deltaTime;

		if (transitionPhase_ == TransitionPhase::FadingOut) {
			float t = std::clamp(transitionTimer_ / half, 0.0f, 1.0f);
			if (currentEffect_ && currentScene_->GetRegistry())
				currentEffect_->UpdateFadingOut(*currentScene_->GetRegistry(), t);

			if (transitionTimer_ >= half) {
				if (currentScene_) currentScene_->OnExit();
				auto it = factories_.find(pendingName_);
				if (it != factories_.end()) {
					currentScene_ = it->second();
					currentScene_->SetName(pendingName_);
					currentScene_->Setup();
					currentScene_->OnEnter();
				}
				if (currentEffect_ && currentScene_->GetRegistry())
					currentEffect_->OnFadingInStart(*currentScene_->GetRegistry());

				transitionPhase_ = TransitionPhase::FadingIn;
				transitionTimer_ = 0.0f;
			}
		} else if (transitionPhase_ == TransitionPhase::FadingIn) {
			float t = std::clamp(transitionTimer_ / half, 0.0f, 1.0f);
			if (currentEffect_ && currentScene_->GetRegistry())
				currentEffect_->UpdateFadingIn(*currentScene_->GetRegistry(), t);

			if (transitionTimer_ >= half) {
				if (currentEffect_ && currentScene_->GetRegistry())
					currentEffect_->OnFinished(*currentScene_->GetRegistry());
				currentEffect_.reset();

				isTransitioning_ = false;
				transitionPhase_ = TransitionPhase::None;
				transitionTimer_ = 0.0f;
				isChanging_ = false;
			}
		}

		if (currentScene_) currentScene_->Update(ctx, deltaTime);
		return;
	}
	if (currentScene_) currentScene_->Update(ctx, deltaTime);
}
}
}
