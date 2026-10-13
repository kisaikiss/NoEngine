#include "SystemManager.h"
#include "SystemSceneConfig.h"
#include "PlayModeController.h"
#include "../Component/Common/PauseComponent.h"
#include "../../Scene/IScene.h"

namespace NoEngine {
namespace ECS {

namespace {
bool IsPaused(Registry& registry) {
	bool isPause = false;
	for (auto e : registry.View<PauseComponent>()) {
		isPause = registry.GetComponent<PauseComponent>(e)->isPause;
	}
	return isPause;
}
}

bool SystemManager::AddSystemByName(const std::string& name) {
	auto system = SystemCatalog::Get().Create(name);
	if (!system) return false;
	entries_.push_back({ name, std::move(system) });
	return true;
}

void SystemManager::RemoveSystem(size_t index) {
	if (index >= entries_.size()) return;
	entries_.erase(entries_.begin() + index);
}

void SystemManager::MoveSystem(size_t from, size_t to) {
	if (from >= entries_.size() || to >= entries_.size() || from == to) return;
	Entry entry = std::move(entries_[from]);
	entries_.erase(entries_.begin() + from);
	entries_.insert(entries_.begin() + to, std::move(entry));
}

bool SystemManager::HasSystem(const std::string& name) const {
	return std::any_of(entries_.begin(), entries_.end(),
		[&](const Entry& e) { return e.name == name; });
}

std::vector<std::string> SystemManager::GetSystemNames() const {
	std::vector<std::string> names;
	names.reserve(entries_.size());
	for (const auto& e : entries_) names.push_back(e.name);
	return names;
}

void SystemManager::LoadConfiguration(Registry& registry) {
	const std::string sceneName = Scene::GetCurrentSceneName(registry);
	if (sceneName.empty() || sceneName == configuredSceneName_) return;
	configuredSceneName_ = sceneName;

	auto names = SystemSceneConfig::Load(sceneName);
	if (!names) return; // ファイルなし/不正ならコード側で組んだ構成のまま

	std::vector<Entry> configured;
	configured.reserve(names->size());
	for (auto& name : *names) {
		auto system = SystemCatalog::Get().Create(name);
		if (!system) return;
		configured.push_back({ std::move(name), std::move(system) });
	}
	entries_ = std::move(configured);
}

void SystemManager::SaveConfiguration(Registry& registry) {
	SystemSceneConfig::Save(Scene::GetCurrentSceneName(registry), GetSystemNames());
}

void SystemManager::UpdateAll(ComputeContext& ctx, Registry& registry, float deltaTime) {
	LoadConfiguration(registry);

	const bool isPause = IsPaused(registry);
	const bool gameStopped = PlayModeController::IsGameStopped();

	for (auto& entry : entries_) {
		ISystem& system = *entry.system;
		if (gameStopped && system.GetStopInGameStop()) continue;
		if (isPause && system.GetStopInPause()) continue;
		system.Update(ctx, registry, deltaTime);
	}

#ifdef USE_IMGUI
	PlayModeController::Update(registry);
	gameControllerPanel_.Draw(registry);
	if (systemListPanel_.Draw(*this)) {
		SaveConfiguration(registry);
	}
#endif // USE_IMGUI
}

}
}