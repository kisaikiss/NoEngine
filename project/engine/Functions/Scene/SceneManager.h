#pragma once
#include "IScene.h"

#include <unordered_map>
#include <functional>
#include <memory>
#include <string>

namespace NoEngine {
namespace Scene {
class SceneManager {
public:
	using SceneFactory = std::function<std::unique_ptr<IScene>()>;

	void RegisterScene(const std::string& name, SceneFactory factory) { factories_[name] = factory; }

	// immediate == true : 即時切替
	// immediate == false: CircleScale 遷移開始
	void ChangeScene(const std::string& name, bool immediate = true);

	void Update(ComputeContext& ctx, float deltaTime);

	ECS::Registry* GetRegistry() { return currentScene_ ? currentScene_->GetRegistry() : nullptr; }

private:
	void CreateCircleOverlay(float initialAlpha, float initialScale);
	void UpdateOverlay(float alpha, float scale);
	void DestroyOverlay();

private:
	std::unordered_map<std::string, SceneFactory> factories_;
	std::unique_ptr<IScene> currentScene_;

	// 遷移状態
	std::string pendingName_;
	bool isChanging_ = false;
	bool isTransitioning_ = false;

	enum class TransitionPhase { None, FadingOut, Loading, FadingIn };
	TransitionPhase transitionPhase_ = TransitionPhase::None;

	float transitionTimer_ = 0.0f;
	float transitionDuration_ = 1.5f;

	ECS::Entity overlayEntity_ = 0;
};

} // namespace Scene
} // namespace NoEngine
