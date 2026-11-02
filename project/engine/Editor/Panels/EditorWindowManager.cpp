#include "stdafx.h"
#include "EditorWindowManager.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

namespace NoEngine::Editor {

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
