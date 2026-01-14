#include "SceneManager.h"

namespace NoEngine {
void Scene::SceneManager::ChangeScene(const std::string& name) {
	auto it = factories_.find(name);
	if (it == factories_.end()) return;

	if (currentScene_) {
		currentScene_->OnExit();
	}

	currentScene_ = it->second();
	currentScene_->Setup();
	currentScene_->OnEnter();
}
}