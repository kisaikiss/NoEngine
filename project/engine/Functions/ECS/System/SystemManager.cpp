#include "SystemManager.h"
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif // USE_IMGUI



namespace NoEngine {
namespace ECS {
void SystemManager::UpdateAll(ComputeContext& ctx, Registry& registry, float deltaTime) {
	for (auto& system : systems_) {
		if (!gameStop_ || !system->GetStopInGameStop()) {
			system->Update(ctx, registry, deltaTime);
		}
		
	}
#ifdef USE_IMGUI
	ImGui::Begin("Game");
	if (ImGui::Button("||")) {
		gameStop_ = true;
	}
	ImGui::SameLine();
	if (ImGui::Button(">")) {
		gameStop_ = false;
	}
	ImGui::End();
#endif // USE_IMGUI

}
}
}