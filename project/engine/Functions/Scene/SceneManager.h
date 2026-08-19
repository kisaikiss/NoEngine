#pragma once
#include "IScene.h"
#include "ITransitionEffect.h"
#include "../ECS/Event/SceneChangeEvent.h"

namespace NoEngine {
namespace Scene {
class SceneManager {
public:
	using SceneFactory = std::function<std::unique_ptr<IScene>()>;
	using TransitionFactory = std::function<std::unique_ptr<ITransitionEffect>()>;

	SceneManager(); // 組み込み演出("CircleScale","Fade")を登録する

	void RegisterScene(const std::string& name, SceneFactory factory) { factories_[name] = factory; }

	void RegisterTransitionEffect(const std::string& name, TransitionFactory factory) {	transitionFactories_[name] = std::move(factory); }

	void ChangeScene(const std::string& name, bool immediate = true,
		Event::SceneTransitionType transitionType = Event::SceneTransitionType::kCircleScale,
		const std::string& customTransitionName = "");

	void Update(ComputeContext& ctx, float deltaTime);
	ECS::Registry* GetRegistry() { return currentScene_ ? currentScene_->GetRegistry() : nullptr; }

private:
	std::unique_ptr<ITransitionEffect> CreateTransitionEffect(Event::SceneTransitionType type, const std::string& customName);

	std::unordered_map<std::string, SceneFactory> factories_;
	std::unordered_map<std::string, TransitionFactory> transitionFactories_;
	std::unique_ptr<IScene> currentScene_;

	std::string pendingName_;
	bool isChanging_ = false;
	bool isTransitioning_ = false;
	enum class TransitionPhase { None, FadingOut, Loading, FadingIn };
	TransitionPhase transitionPhase_ = TransitionPhase::None;
	float transitionTimer_ = 0.0f;
	float transitionDuration_ = 1.5f;

	std::unique_ptr<ITransitionEffect> currentEffect_;
};
} // namespace Scene
} // namespace NoEngine
