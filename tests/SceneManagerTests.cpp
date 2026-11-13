#include "engine/NoEngine.h"
#include "engine/Functions/ECS/System/PlayModeController.h"
#include "engine/Editor/Commands/EditorCommandOperator.h"
#include "engine/Editor/DataDriven/SceneSerializer.h"
#include <iostream>

namespace {
int checks = 0;
void Check(bool ok, const char* message) {
	++checks;
	if (!ok) throw std::runtime_error(message);
}

void Write(const std::string& name, const nlohmann::json& data) {
	std::ofstream out("resources/game/Scenes/" + name + ".json");
	out << data.dump(4);
}

nlohmann::json Read(const std::string& path) {
	std::ifstream file(path);
	nlohmann::json data;
	file >> data;
	return data;
}

class HistoryMarker : public NoEngine::Command::ICommand {
public:
	HistoryMarker() : ICommand("Scene regression test") {}
	void Execute() override {}
	void Undo() override {}
};

void CheckFixtures() {
	using namespace NoEngine;
	std::filesystem::create_directories("resources/game/Scenes");
	nlohmann::json data = {
		{ "entities", {{ "Anchor", {{ "components", {{ "TransformComponent", {{ "translate", { 1.0f, 2.0f, 3.0f } }}} }}}}} },
		{ "systems", nlohmann::json::array() },
		{ "renderPasses", {{{ "name", "Test pass" }, { "settings", nlohmann::json::object() }}} }
	};
	Write("Active", data);
	Write("Malformed", data);
	Write("Unknown", data);
	Write("BadEntity", data);
	Write("Duplicate", data);
	Write("Legacy", { { "entities", nlohmann::json::object() } });
	{
		std::ofstream out("resources/game/application.json");
		out << nlohmann::json{ { "scenes", { "Active", "Malformed", "Unknown", "BadEntity", "Duplicate", "Legacy" } }, { "removedScenes", nlohmann::json::array() } };
	}
	Scene::SceneManager manager;
	Check(manager.ChangeScene("Active"), "load without EditSystem");
	auto* original = manager.GetRegistry();
	auto anchor = Editor::FindEntityByName(*original, "Anchor");
	Check(anchor != ECS::INVALID_ENTITY, "entities loaded before the first update");
	Check(original->GetComponent<Component::TransformComponent>(anchor)->translate.y == 2.0f, "component data loaded");
	Check(Editor::FindEntityByName(*original, "MainCamera") == ECS::INVALID_ENTITY, "loading does not inject cameras");
	Editor::EditorCommandOperator::AddCommand(std::make_unique<HistoryMarker>());
	Check(Editor::EditorCommandOperator::CanUndo(), "history marker added");
	{ std::ofstream out("resources/game/Scenes/Malformed.json"); out << "{"; }
	auto bad = data;
	bad["systems"] = { "MissingSystem" }; Write("Unknown", bad);
	bad = data; bad["entities"]["Anchor"]["components"]["TransformComponent"]["translate"] = "invalid"; Write("BadEntity", bad);
	bad = data; bad["systems"] = { "NumberDisplaySystem", "NumberDisplaySystem" }; Write("Duplicate", bad);
	for (const auto* name : { "Malformed", "Unknown", "BadEntity", "Duplicate", "Missing", "../Active" }) {
		Check(!manager.ChangeScene(name), "invalid target rejected");
		Check(manager.GetRegistry() == original, "failed load preserves active scene");
		Check(Editor::EditorCommandOperator::CanUndo(), "failed load preserves history");
	}
	Check(!manager.ChangeScene("Unknown", false), "invalid target rejected before fade");
	Check(manager.ChangeScene("Legacy"), "legacy scene without systems remains readable");
	Check(!Editor::EditorCommandOperator::CanUndo(), "successful switch clears history");
	Check(manager.CreateScene("Created"), "create scene");
	auto created = Read("resources/game/Scenes/Created.json");
	Check(created["systems"] == nlohmann::json::array({ "EditSystem", "DrawManipulatorSystem" }), "new scene system template saved");
	Check(created["entities"].contains("MainCamera") && created["entities"].contains("DebugCamera"), "camera template saved");
	created["entities"].erase("MainCamera"); Write("Created", created);
	Check(manager.ChangeScene("Created"), "reload edited scene");
	Check(Editor::FindEntityByName(*manager.GetRegistry(), "MainCamera") == ECS::INVALID_ENTITY, "deleted camera stays deleted");
	Check(!manager.CreateScene("Created") && !manager.CreateScene("../Escape"), "duplicate and unsafe names rejected");
	Check(manager.ChangeScene("Active"), "return to scene without editor systems");
	anchor = Editor::FindEntityByName(*manager.GetRegistry(), "Anchor");
	manager.GetRegistry()->GetComponent<Component::TransformComponent>(anchor)->translate.y = 42.0f;
	Check(manager.CopyScene("Copied"), "copy scene");
	auto copied = Read("resources/game/Scenes/Copied.json");
	Check(copied["entities"]["Anchor"]["components"]["TransformComponent"]["translate"][1] == 42.0f, "copy includes unsaved entity edits");
	Check(copied["systems"].empty(), "copy preserves an empty system list");
	Check(copied["renderPasses"] == data["renderPasses"], "copy preserves render settings");
	Check(manager.DeleteScene("Copied"), "delete current scene with valid fallback");
	Check(!std::filesystem::exists("resources/game/Scenes/Copied.json"), "deleted scene file removed");
	Check(Scene::GetCurrentSceneName(*manager.GetRegistry()) != "Copied", "fallback scene activated");
	Check(!manager.GetSceneNames().contains("Copied"), "deleted scene removed from index");
	Check(!Read("resources/game/application.json").contains("removedScenes"), "index contains only data-defined scenes");

	// 切り替え後のStopでも、開始時の未保存データを復元できることを確認する。
	Check(manager.ChangeScene("Active"), "prepare play mode start scene");
	anchor = Editor::FindEntityByName(*manager.GetRegistry(), "Anchor");
	manager.GetRegistry()->GetComponent<Component::TransformComponent>(anchor)->translate.y = 77.0f;
	ECS::PlayModeController::Play(*manager.GetRegistry());
	Check(manager.ChangeScene("Created"), "change scene while playing");
	ECS::PlayModeController::Stop(*manager.GetRegistry());
	auto restore = ECS::PlayModeController::TakeSceneRestoreRequest();
	Check(restore == "Active" && manager.ChangeScene(*restore), "restore starts from original scene");
	ECS::PlayModeController::Update(*manager.GetRegistry());
	anchor = Editor::FindEntityByName(*manager.GetRegistry(), "Anchor");
	Check(manager.GetRegistry()->GetComponent<Component::TransformComponent>(anchor)->translate.y == 77.0f, "play snapshot survives scene replacement");
	ECS::PlayModeController::Shutdown();

	// 不正な候補しか残らない場合は、使用中のシーンもファイルも消さない。
	Check(manager.DeleteScene("Created") && manager.DeleteScene("Legacy"), "remove valid fallback candidates");
	Check(!manager.DeleteScene("Active"), "delete rejected when all fallback files are invalid");
	Check(std::filesystem::exists("resources/game/Scenes/Active.json"), "failed delete preserves scene file");
	for (const auto* name : { "Malformed", "Unknown", "BadEntity", "Duplicate" }) Check(manager.DeleteScene(name), "remove invalid inactive scene");
	Check(!manager.DeleteScene("Active"), "last scene cannot be deleted");
	Scene::SceneManager reopened;
	Check(reopened.GetSceneNames() == std::set<std::string>{ "Active" }, "scene index survives reopening");
}
}

int main(int argc, char** argv) {
	try {
		if (argc != 3) throw std::runtime_error("Expected isolated fixture directory and project directory");
		const auto fixtures = std::filesystem::absolute(argv[1]);
		const auto project = std::filesystem::absolute(argv[2]);
		std::filesystem::create_directories(fixtures);
		std::filesystem::current_path(fixtures);
		CheckFixtures();
		{
			NoEngine::ECS::SystemManager systems;
			const std::vector<std::string> order{ "NumberDisplaySystem", "GameClearInputSystem" };
			Check(systems.SetSystems(order) && systems.GetSystemNames() == order, "configured system order preserved");
			Check(!systems.SetSystems({ "NumberDisplaySystem", "MissingSystem" }) && systems.GetSystemNames() == order, "failed configuration leaves systems intact");
			Check(systems.SetSystems({}) && systems.GetSystemNames().empty(), "empty configuration is applied explicitly");
		}
		std::filesystem::current_path(project);
		// GPUを使うSystemの生成は避け、実シーンの配置復元と登録名の照合を確認する。
		{
			NoEngine::Scene::SceneManager manager;
			for (const auto& name : manager.GetSceneNames()) {
				const auto data = Read("resources/game/Scenes/" + name + ".json");
				for (const auto& system : data.at("systems")) {
					Check(NoEngine::ECS::SystemCatalog::Get().Contains(system.get<std::string>()), "production scene system registered");
				}
				NoEngine::ECS::Registry registry;
				NoEngine::Editor::LoadScene(registry, data);
				for (const auto& [entityName, entityData] : data.at("entities").items()) {
					Check(NoEngine::Editor::FindEntityByName(registry, entityName) != NoEngine::ECS::INVALID_ENTITY, "production scene entity restored");
				}
			}
		}
		std::cout << checks << " scene checks passed\n";
		return 0;
	} catch (const std::exception& error) {
		std::cerr << "FAIL: " << error.what() << '\n';
		return 1;
	}
}
