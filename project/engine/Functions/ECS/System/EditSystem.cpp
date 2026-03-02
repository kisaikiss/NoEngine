#include "EditSystem.h"
#include "engine/Editor/EditUtils.h"
#include "engine/Editor/DataDriven/SceneSerializer.h"
#include "engine/Functions/Debug/Logger/Log.h"
#include "engine/Functions/Scene/SceneNameComponent.h"
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif // USE_IMGUI

namespace {
const std::string skDirectoryPath = "resources/game/Scenes/";
}

namespace NoEngine {
namespace ECS {
void EditSystem::Update(Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
    if (!FirstLoaded_) {
        LoadFile(registry);
        FirstLoaded_ = true;
    }
#ifdef USE_IMGUI
    auto view = registry.View<Editor::EditTag>();

    for (auto e : view) {
        auto* tag = registry.GetComponent<Editor::EditTag>(e);
        ImGui::Begin(tag->name.c_str());

        DrawComponentUI(registry, e);

        ImGui::End();
    }

    ImGui::Begin("file");
    if (ImGui::Button("SaveAll")) {
        SaveFile(registry, Editor::SaveScene(registry));
    }
    if (ImGui::Button("LoadAll")) {
        LoadFile(registry);
    }
    ImGui::End();

#else
	static_cast<void>(registry);
#endif // USE_IMGUI
}

void EditSystem::SaveFile(Registry& registry, nlohmann::json j) {
    std::filesystem::path dir(skDirectoryPath);
    if (!std::filesystem::exists(dir)) {
        std::filesystem::create_directory(dir);
    }

    std::string sceneName;
    auto view = registry.View<SceneNameComponent>();
    for (auto entity : view) {
        auto* nameComp = registry.GetComponent<SceneNameComponent>(entity);
        sceneName = nameComp->GetName();
    }
    std::string filePath = skDirectoryPath + sceneName + ".json";
    std::ofstream file(filePath);
    if (!file.is_open()) {
        Log::DebugPrint("Failed to open file for writing", VerbosityLevel::kError);
        return;
    }

    file << j.dump(4);
}

void EditSystem::LoadFile(Registry& registry) {
    std::string sceneName;
    auto view = registry.View<SceneNameComponent>();
    for (auto entity : view) {
        auto* nameComp = registry.GetComponent<SceneNameComponent>(entity);
        sceneName = nameComp->GetName();
    }
    std::string filePath = skDirectoryPath + sceneName + ".json";

    std::ifstream file(filePath);
    if (!file.is_open()) {
        Log::DebugPrint("Failed to open scene file:" + filePath, VerbosityLevel::kError);
        return;
    }

    nlohmann::json scene;
    file >> scene;

    Editor::LoadScene(registry, scene);
}
}
}