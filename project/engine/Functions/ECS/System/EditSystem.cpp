#include "EditSystem.h"
#include "engine/Editor/EditUtils.h"
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif // USE_IMGUI


namespace NoEngine {
namespace ECS {
void EditSystem::Update(Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
#ifdef USE_IMGUI
    auto view = registry.View<Editor::EditTag>();

    for (auto e : view) {
        auto* tag = registry.GetComponent<Editor::EditTag>(e);
        ImGui::Begin(tag->name.c_str());

        DrawComponentUI(registry, e);

        ImGui::End();
    }

#else
	static_cast<void>(registry);
#endif // USE_IMGUI
}
}
}