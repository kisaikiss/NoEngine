#include "SystemManager.h"
#include "../Component/PauseComponent.h"
#include "../Event/SceneChangeEvent.h"
#include "../../Scene/SceneNameComponent.h"
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif // USE_IMGUI



namespace NoEngine {
namespace ECS {

SystemManager::SystemManager() {
#ifdef USE_IMGUI
	gameStop_ = true;
#endif // USE_IMGUI

}

void SystemManager::UpdateAll(ComputeContext& ctx, Registry& registry, float deltaTime) {
	auto pauseView = registry.View<PauseComponent>();
	bool isPause = false;
	for(auto entity : pauseView){
		auto* pauseComp = registry.GetComponent<PauseComponent>(entity);
		isPause = pauseComp->isPause;
	}


	for (auto& system : systems_) {
		if (!gameStop_ || !system->GetStopInGameStop()) {
			if (!isPause || !system->GetStopInPause()) {
				system->Update(ctx, registry, deltaTime);
			}
		}
		
	}
#ifdef USE_IMGUI
	ImGui::Begin("Game");
	if (ImGui::Button("■")) {
		auto sceneView = registry.View<SceneNameComponent>();
		for (auto e : sceneView) {
			auto* sceneName = registry.GetComponent<SceneNameComponent>(e);
			Event::SceneChangeEvent event;
			event.nextScene = sceneName->GetName();
			registry.EmitEvent(event);
		}

	}
	ImGui::SameLine();
	if (ImGui::Button(">")) {
		gameStop_ = false;
	}
	ImGui::SameLine();
	if (ImGui::Button("||")) {
		gameStop_ = true;
	}

	ImGui::End();
#endif // USE_IMGUI

}
}
}