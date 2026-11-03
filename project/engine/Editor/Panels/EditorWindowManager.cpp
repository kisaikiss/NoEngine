#include "stdafx.h"
#include "EditorWindowManager.h"
#include "externals/nlohmann/json.hpp"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

namespace NoEngine::Editor {
namespace {
constexpr const char* kSettingsPath = "editor-windows.json";
// 表示名を変更しても保存済みの設定を引き継げる固定キー。
constexpr std::array<const char*, 3> kWindowKeys{"grid", "systems", "renderPasses"};
static_assert(kWindowKeys.size() == static_cast<std::size_t>(EditorWindowId::Count));
}

void EditorWindowManager::LoadSettings() {
    ResetToDefaults();
    std::ifstream input(kSettingsPath);
    if (!input) return;
    try {
        const auto data = nlohmann::json::parse(input);
        if (!data.is_object()) return;
        for (std::size_t i = 0; i < windows_.size(); ++i) {
            const auto it = data.find(kWindowKeys[i]);
            if (it != data.end() && it->is_boolean()) windows_[i].open = it->get<bool>();
        }
    } catch (const nlohmann::json::exception&) {
        LogWarning("Failed to load editor window settings. Using defaults.");
    }
}

void EditorWindowManager::SaveSettings() const {
    nlohmann::json data = nlohmann::json::object();
    for (std::size_t i = 0; i < windows_.size(); ++i) data[kWindowKeys[i]] = windows_[i].open;
    std::ofstream output(kSettingsPath);
    if (output) {
        output << data.dump(4) << '\n';
        output.flush();
    }
    if (!output) LogWarning("Failed to save editor window settings.");
}

EditorWindowManager& EditorWindowManager::GetInstance() {
    static EditorWindowManager instance;
    return instance;
}

EditorWindowState& EditorWindowManager::Get(EditorWindowId id) {
    return windows_.at(static_cast<std::size_t>(id));
}

void EditorWindowManager::DrawMenu() {
#ifdef USE_IMGUI
    if (ImGui::BeginMenu("Window")) {
        for (auto& state : windows_) {
            ImGui::MenuItem(state.label, nullptr, &state.open);
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Reset window visibility")) {
            ResetToDefaults();
        }
        ImGui::EndMenu();
    }
#endif
}

void EditorWindowManager::ResetToDefaults() {
    for (auto& state : windows_) {
        state.open = state.defaultOpen;
    }
}

}
