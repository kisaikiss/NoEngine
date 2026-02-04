#pragma once
#include "IScene.h"
namespace NoEngine {
namespace Scene {
	class SceneManager
	{
	public:
		using SceneFactory = std::function<std::unique_ptr<IScene>()>;

		void RegisterScene(const std::string& name, SceneFactory factory) { factories_[name] = factory; }

		// immediate == true : 即時切替
		// immediate == false: フェード遷移開始
		void ChangeScene(const std::string& name, bool immediate = true);

		// Scene の Update は SceneManager 側で遷移制御を行うため実装を cpp に移動
		void Update(float deltaTime);

		ECS::Registry* GetRegistry() { return currentScene_ ? currentScene_->GetRegistry() : nullptr; }

		CameraBase* GetCamera() { return currentScene_ ? currentScene_->GetCamera() : nullptr; }

	private:
		std::unordered_map<std::string, SceneFactory> factories_;
		std::unique_ptr<IScene> currentScene_;

		// フェード遷移用状態
		std::string pendingName_;
		bool isChanging_ = false;
		bool isTransitioning_ = false;
		enum class TransitionPhase { None, FadingOut, Loading, FadingIn };
		TransitionPhase transitionPhase_ = TransitionPhase::None;
		float transitionTimer_ = 0.0f;
		float transitionDuration_ = 1.5f;
		ECS::Entity overlayEntity_ = 0;
	};
}
}

