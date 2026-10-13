#include "stdafx.h"
#include "GameControllerPanel.h"
#include "engine/Functions/ECS/System/PlayModeController.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif // USE_IMGUI

namespace NoEngine {
namespace Editor {

void GameControllerPanel::Draw(ECS::Registry& registry) {
#ifdef USE_IMGUI
	ImGui::Begin("GameController", nullptr, ImGuiWindowFlags_NoTitleBar);

	// ボタンを中央寄せする
	const float windowWidth = ImGui::GetWindowSize().x;
	const float itemWidth = ImGui::CalcTextSize("Button").x + ImGui::GetStyle().FramePadding.x * 2;
	ImGui::SetCursorPosX(windowWidth * 0.5f - itemWidth);

	if (ImGui::Button("■")) {
		ECS::PlayModeController::Stop(registry);
	}
	ImGui::SameLine();
	if (ImGui::Button(">")) {
		ECS::PlayModeController::Play(registry);
	}
	ImGui::SameLine();
	if (ImGui::Button("||")) {
		ECS::PlayModeController::Pause();
	}

	ImGui::End();
#else
	static_cast<void>(registry);
#endif // USE_IMGUI
}

}
}