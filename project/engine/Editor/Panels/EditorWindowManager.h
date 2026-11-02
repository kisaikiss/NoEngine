#pragma once

namespace NoEngine::Editor {

enum class EditorWindowId {
    Grid,
    Systems,
    RenderPasses,
    Count
};

struct EditorWindowState {
    const char* label;
    bool open;
    bool defaultOpen;
};

// 表示状態はシーンや各パネルの寿命から独立して保持する。
class EditorWindowManager {
public:
    static EditorWindowManager& GetInstance();

    EditorWindowState& Get(EditorWindowId id);
    void DrawMenu();
    void ResetToDefaults();

private:
    EditorWindowManager() = default;
    EditorWindowManager(const EditorWindowManager&) = delete;
    EditorWindowManager& operator=(const EditorWindowManager&) = delete;

    std::array<EditorWindowState, static_cast<std::size_t>(EditorWindowId::Count)> windows_{{
        {"Grid", true, true},
        {"Systems", true, true},
        {"Render Passes", true, true},
    }};
};

}
