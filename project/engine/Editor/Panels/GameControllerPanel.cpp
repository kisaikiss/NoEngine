#include "stdafx.h"
#include "GameControllerPanel.h"
#include "engine/Functions/ECS/System/PlayModeController.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif // USE_IMGUI

namespace NoEngine {
namespace Editor {

void GameControllerPanel::Draw(ECS::Registry& registry, ECS::SystemManager* systems) {
#ifdef USE_IMGUI
	if (ImGui::Begin("GameController", nullptr, ImGuiWindowFlags_NoTitleBar)) {
		using Controller = ECS::PlayModeController;
		const auto state = Controller::GetState();
		const bool editing = state == Controller::State::kEditing;
		const bool playing = state == Controller::State::kPlaying;
		const bool paused = state == Controller::State::kPaused;

		const auto& style = ImGui::GetStyle();
		const float padding = style.FramePadding.x * 2.0f;
		const float stopWidth = ImGui::CalcTextSize("■").x + padding;
		const float playWidth = std::max(ImGui::CalcTextSize(">").x, ImGui::CalcTextSize(">").x) + padding;
		const float pauseWidth = ImGui::CalcTextSize("||").x + padding;
		const float rowWidth = stopWidth + playWidth + pauseWidth + style.ItemSpacing.x * 2.0f;
		const auto center = [](float width) {
			const float offset = (ImGui::GetContentRegionAvail().x - width) * 0.5f;
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (offset > 0.0f ? offset : 0.0f));
		};

		const auto button = [](const char* label, float width, bool enabled, const char* tooltip) {
			ImGui::BeginDisabled(!enabled);
			const bool clicked = ImGui::Button(label, ImVec2(width, 0.0f));
			ImGui::EndDisabled();
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
				ImGui::SetTooltip("%s", tooltip);
			}
			return clicked;
		};

		center(rowWidth);
		if (button("■", stopWidth, !editing, "再生を停止し、再生開始前のシーン状態に戻します。")) {
			Controller::Stop(registry, systems);
		}
		ImGui::SameLine();
		if (button(paused ? ">###Play" : ">###Play", playWidth, !playing,
			paused ? "一時停止した位置から再開します。" : "現在のシーンを再生します。")) {
			Controller::Play(registry, systems);
		}
		ImGui::SameLine();
		if (button("||", pauseWidth, playing, "ゲームの進行を一時停止します。")) {
			Controller::Pause();
		}

		const auto currentState = Controller::GetState();
		const char* status = "編集中";
		if (currentState == Controller::State::kPlaying) {
			status = "再生中";
		} else if (currentState == Controller::State::kPaused) {
			status = "一時停止中";
		}
		center(ImGui::CalcTextSize(status).x);
		ImGui::TextUnformatted(status);
	}

	ImGui::End();
#else
	static_cast<void>(registry);
	static_cast<void>(systems);
#endif // USE_IMGUI
}

}
}
