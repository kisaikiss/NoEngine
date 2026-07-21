#include "SystemManager.h"
#include "../Component/Common/PauseComponent.h"
#include "../Event/SceneChangeEvent.h"
#include "../../Scene/SceneNameComponent.h"
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif // USE_IMGUI



namespace NoEngine {
namespace ECS {

namespace {
#ifdef USE_IMGUI
bool sGameStop = true;
#else
bool sGameStop = false;
#endif // USE_IMGUI
}

SystemManager::SystemManager() {}

void SystemManager::UpdateAll(ComputeContext& ctx, Registry& registry, float deltaTime) {
	auto pauseView = registry.View<PauseComponent>();
	bool isPause = false;
	for(auto entity : pauseView){
		auto* pauseComp = registry.GetComponent<PauseComponent>(entity);
		isPause = pauseComp->isPause;
	}


	for (auto& system : systems_) {
		if (!sGameStop || !system->GetStopInGameStop()) {
			if (!isPause || !system->GetStopInPause()) {
				system->Update(ctx, registry, deltaTime);
			}
		}
		
	}
#ifdef USE_IMGUI


	ImGui::Begin("GameController", nullptr, ImGuiWindowFlags_NoTitleBar);
	float windowWidth = ImGui::GetWindowSize().x;
	float itemWidth = ImGui::CalcTextSize("Button").x + ImGui::GetStyle().FramePadding.x * 2;

	ImGui::SetCursorPosX(windowWidth * 0.5f - itemWidth);
	if (ImGui::Button("■")) {
		sGameStop = true;
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
		sGameStop = false;
	}
	ImGui::SameLine();
	if (ImGui::Button("||")) {
		sGameStop = true;
	}

	ImGui::End();
#endif // USE_IMGUI

}
}
}