#pragma once
#include "Scene.h"
#include "ITransitionEffect.h"
#include "../ECS/Event/SceneChangeEvent.h"
#include <set>
#include "externals/nlohmann/json.hpp"

namespace NoEngine {
namespace Scene {
class SceneManager {
public:
	using TransitionFactory = std::function<std::unique_ptr<ITransitionEffect>()>;

	SceneManager();
	~SceneManager();

	bool CreateScene(const std::string& name);
	bool CopyScene(const std::string& name);
	bool DeleteScene(const std::string& name);
	const std::set<std::string>& GetSceneNames() const { return sceneNames_; }

	void RegisterTransitionEffect(const std::string& name, TransitionFactory factory) {	transitionFactories_[name] = std::move(factory); }

	bool ChangeScene(const std::string& name, bool immediate = true,
		Event::SceneTransitionType transitionType = Event::SceneTransitionType::kCircleScale,
		const std::string& customTransitionName = "");

	void Update(ComputeContext& ctx, float deltaTime);
	ECS::Registry* GetRegistry() { return currentScene_ ? &currentScene_->GetRegistry() : nullptr; }

private:
	std::unique_ptr<ITransitionEffect> CreateTransitionEffect(Event::SceneTransitionType type, const std::string& customName);

	std::unique_ptr<Scene> LoadScene(const std::string& name);
	void CommitScene(std::unique_ptr<Scene> next);
	void ResetTransition(bool finishEffect = true);
	bool ReloadScene();
	bool SaveSceneList(const std::set<std::string>& names);
	bool SaveNewScene(const std::string& name, const nlohmann::json& data);
	void DrawSceneMenu();
	std::set<std::string> sceneNames_;
	std::unordered_map<std::string, TransitionFactory> transitionFactories_;
	std::unique_ptr<Scene> currentScene_;
	std::unique_ptr<Scene> pendingScene_;

	enum class TransitionPhase { None, FadingOut, FadingIn };
	TransitionPhase transitionPhase_ = TransitionPhase::None;
	float transitionTimer_ = 0.0f;
	float transitionDuration_ = 1.5f;

	std::unique_ptr<ITransitionEffect> currentEffect_;
};
} // namespace Scene
} // namespace NoEngine
