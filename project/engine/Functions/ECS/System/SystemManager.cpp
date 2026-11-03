#include "SystemManager.h"
#include "SystemSceneConfig.h"
#include "PlayModeController.h"
#include "../Component/Common/PauseComponent.h"
#include "../../Scene/IScene.h"
#include "engine/Editor/Commands/EditorCommandOperator.h"

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

// 追加・削除した実体を履歴側に預け、Undo/Redoで内部状態を失わずに戻す。
class SystemManager::EditCommand : public Command::ICommand {
public:
	enum class Kind { Add, Remove, Move };

	EditCommand(SystemManager& manager, Kind kind, size_t from, size_t to, Entry held = {})
		: ICommand(kind == Kind::Add ? "Add system" : kind == Kind::Remove ? "Remove system" : "Move system"),
		manager_(manager), lifetime_(manager.editLifetime_), kind_(kind), from_(from), to_(to),
		held_(std::move(held)), selectionBefore_(manager.systemListPanel_.GetSelectedIndex()) {}

	void Execute() override {
		if (lifetime_.expired()) return;
		switch (kind_) {
		case Kind::Add:
			manager_.entries_.insert(manager_.entries_.begin() + to_, std::move(held_));
			manager_.systemListPanel_.Select(static_cast<int>(to_));
			break;
		case Kind::Remove:
			held_ = std::move(manager_.entries_[from_]);
			manager_.entries_.erase(manager_.entries_.begin() + from_);
			manager_.systemListPanel_.Select(manager_.entries_.empty() ? -1 :
				static_cast<int>(std::min(from_, manager_.entries_.size() - 1)));
			break;
		case Kind::Move:
			manager_.MoveEntry(from_, to_);
			manager_.systemListPanel_.Select(static_cast<int>(to_));
			break;
		}
		manager_.configurationDirty_ = true;
	}

	void Undo() override {
		if (lifetime_.expired()) return;
		switch (kind_) {
		case Kind::Add:
			held_ = std::move(manager_.entries_[to_]);
			manager_.entries_.erase(manager_.entries_.begin() + to_);
			break;
		case Kind::Remove:
			manager_.entries_.insert(manager_.entries_.begin() + from_, std::move(held_));
			break;
		case Kind::Move:
			manager_.MoveEntry(to_, from_);
			break;
		}
		manager_.systemListPanel_.Select(selectionBefore_);
		manager_.configurationDirty_ = true;
	}

private:
	SystemManager& manager_;
	std::weak_ptr<int> lifetime_;
	Kind kind_;
	size_t from_;
	size_t to_;
	Entry held_;
	int selectionBefore_;
};

bool SystemManager::EditAddSystem(const std::string& name, size_t insertionIndex) {
	if (insertionIndex > entries_.size() || HasSystem(name)) return false;
	auto system = SystemCatalog::Get().Create(name);
	if (!system) return false;
	auto command = std::make_unique<EditCommand>(*this, EditCommand::Kind::Add, 0,
		insertionIndex, Entry{ name, std::move(system) });
	command->Execute();
	Editor::EditorCommandOperator::AddCommand(std::move(command));
	return true;
}

bool SystemManager::EditRemoveSystem(size_t index) {
	if (index >= entries_.size()) return false;
	auto command = std::make_unique<EditCommand>(*this, EditCommand::Kind::Remove, index, 0);
	command->Execute();
	Editor::EditorCommandOperator::AddCommand(std::move(command));
	return true;
}

bool SystemManager::EditMoveSystem(size_t from, size_t to) {
	if (from >= entries_.size() || to >= entries_.size() || from == to) return false;
	auto command = std::make_unique<EditCommand>(*this, EditCommand::Kind::Move, from, to);
	command->Execute();
	Editor::EditorCommandOperator::AddCommand(std::move(command));
	return true;
}

bool SystemManager::AddSystemByName(const std::string& name) {
	auto system = SystemCatalog::Get().Create(name);
	if (!system) return false;
	entries_.push_back({ name, std::move(system) });
	ResetEditorState();
	return true;
}

void SystemManager::RemoveSystem(size_t index) {
	if (index >= entries_.size()) return;
	entries_.erase(entries_.begin() + index);
	ResetEditorState();
}

void SystemManager::MoveSystem(size_t from, size_t to) {
	if (from >= entries_.size() || to >= entries_.size() || from == to) return;
	MoveEntry(from, to);
	ResetEditorState();
}

void SystemManager::MoveEntry(size_t from, size_t to) {
	Entry entry = std::move(entries_[from]);
	entries_.erase(entries_.begin() + from);
	entries_.insert(entries_.begin() + to, std::move(entry));
}

void SystemManager::ResetEditorState() {
	// 履歴を介さず構成が変わった場合、古いインデックスによる操作を無効化する。
	editLifetime_ = std::make_shared<int>(0);
	systemListPanel_.Reset();
	configurationDirty_ = false;
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
	ResetEditorState();

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
	const bool changed = systemListPanel_.Draw(*this);
	// 共通ショートカットによるUndo/Redoも、ウィンドウの開閉に関係なく保存する。
	if (changed || configurationDirty_) {
		SaveConfiguration(registry);
		configurationDirty_ = false;
	}
#endif // USE_IMGUI
}

}
}
