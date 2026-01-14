#pragma once
#include "IScene.h"
namespace NoEngine {
namespace Scene {
class SceneManager {
public:
	using SceneFactory = std::function<std::unique_ptr<IScene>()>;

	void RegisterScene(const std::string& name, SceneFactory factory) { factories_[name] = factory; }

	void ChangeScene(const std::string& name);

	void Update(float deltaTime) { if (currentScene_) currentScene_->Update(deltaTime); }

	ECS::Registry* GetRegistry() { return currentScene_ ? currentScene_->GetRegistry() : nullptr; }

	CameraBase* GetCamera() { return currentScene_ ? currentScene_->GetCamera() : nullptr; }

private:
	std::unordered_map<std::string, SceneFactory> factories_;
	std::unique_ptr<IScene> currentScene_;
};
}
}

