#include "stdafx.h"
#include "RailCameraEditor.h"

#include "application/CommentBout/Component/RailCameraComponent.h"
#include "application/CommentBout/Component/Enemy/EnemyComponent.h"
#include "application/CommentBout/GameTag.h"
#include <cstdio>

void RailCameraEditor::DrawRailCameraContent(No::Registry* registry, No::Entity railCameraEntity, ResetEventRuntimeFn resetEventRuntime)
{
#ifdef USE_IMGUI
	if (!registry || railCameraEntity == No::nullEntity) { return; }
	auto* rail = registry->GetComponent<RailCameraComponent>(railCameraEntity);
	if (!rail) { return; }

	if (ImGui::SliderFloat("速度", &rail->speed, 0.0f, 40.0f) && rail->speed < 0.0f) { rail->speed = 0.0f; }

	if (ImGui::Button(rail->isPlaying ? "一時停止" : "再生")) {
		if (!rail->isFinished || rail->distance < rail->totalLength) {
			rail->isPlaying = !rail->isPlaying;
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("停止")) { rail->distance = 0.0f; rail->isPlaying = false; rail->isFinished = false; }
	ImGui::SameLine();
	if (ImGui::Button("先頭から再生")) {
		rail->distance = 0.0f; rail->isFinished = false; rail->isPlaying = true;
		if (resetEventRuntime) { resetEventRuntime(*rail); }
	}
	ImGui::SameLine();
	if (ImGui::Button("イベント実行状態リセット") && resetEventRuntime) { resetEventRuntime(*rail); }

	float percent = (rail->totalLength > 0.0f) ? (rail->distance / rail->totalLength) * 100.0f : 0.0f;
	if (ImGui::SliderFloat("進行率(%)", &percent, 0.0f, 100.0f) && rail->totalLength > 0.0f) {
		rail->distance = rail->totalLength * (percent / 100.0f);
		rail->isFinished = (rail->distance >= rail->totalLength);
		if (rail->isFinished) { rail->isPlaying = false; }
	}

	if (rail->isFinished)      ImGui::Text("状態: 終了");
	else if (rail->isPlaying)  ImGui::Text("状態: 再生中");
	else                       ImGui::Text("状態: 停止中");
#else
	static_cast<void>(registry); static_cast<void>(railCameraEntity); static_cast<void>(resetEventRuntime);
#endif
}

void RailCameraEditor::DrawRailEditorContent(No::Registry* registry, No::Entity railCameraEntity)
{
#ifdef USE_IMGUI
	if (!registry || railCameraEntity == No::nullEntity) { return; }
	auto* rail = registry->GetComponent<RailCameraComponent>(railCameraEntity);
	if (!rail) { return; }

	if (ImGui::Button("制御点を追加")) {
		No::Vector3 newPoint = { 0.0f, 0.0f, 0.0f };
		if (!rail->controlPoints.empty()) { newPoint = rail->controlPoints.back(); newPoint.z += 2.0f; }
		rail->controlPoints.push_back(newPoint);
		rail->selectedControlPointIndex = static_cast<int>(rail->controlPoints.size()) - 1;
		rail->isLoaded = false; rail->needsRebuildArcLength = true;
	}
	if (ImGui::Button("選択制御点を削除")) {
		const int index = rail->selectedControlPointIndex;
		if (index >= 0 && index < static_cast<int>(rail->controlPoints.size())) {
			rail->controlPoints.erase(rail->controlPoints.begin() + index);
			if (rail->controlPoints.empty()) rail->selectedControlPointIndex = -1;
			else if (rail->selectedControlPointIndex >= static_cast<int>(rail->controlPoints.size()))
				rail->selectedControlPointIndex = static_cast<int>(rail->controlPoints.size()) - 1;
			rail->isLoaded = false; rail->needsRebuildArcLength = true;
		}
	}

	const int selectedIndex = rail->selectedControlPointIndex;
	const bool canAddAfter = (selectedIndex >= 0) && (selectedIndex < static_cast<int>(rail->controlPoints.size()));
	if (!canAddAfter) ImGui::BeginDisabled();
	if (ImGui::Button("選択点の次に追加")) {
		const int insertIndex = selectedIndex + 1;
		const No::Vector3& sel = rail->controlPoints[static_cast<size_t>(selectedIndex)];
		No::Vector3 direction = { 0.0f, 0.0f, 1.0f };
		if ((selectedIndex + 1) < static_cast<int>(rail->controlPoints.size()))
			direction = rail->controlPoints[static_cast<size_t>(selectedIndex + 1)] - sel;
		else if (selectedIndex > 0)
			direction = sel - rail->controlPoints[static_cast<size_t>(selectedIndex - 1)];
		if (direction.LengthSquared() <= 0.000001f) direction = { 0.0f, 0.0f, 1.0f };
		else direction = direction.Normalize();
		rail->controlPoints.insert(rail->controlPoints.begin() + insertIndex, sel + direction * 0.1f);
		rail->selectedControlPointIndex = insertIndex;
		rail->isLoaded = false; rail->needsRebuildArcLength = true;
	}
	if (!canAddAfter) ImGui::EndDisabled();

	if (!rail->controlPoints.empty()) {
		ImGui::Separator();
		ImGui::Text("Control Points");
		ImGui::BeginChild("ControlPointList", ImVec2(0.0f, 200.0f), true);
		for (int i = 0; i < static_cast<int>(rail->controlPoints.size()); ++i) {
			char label[48];
			std::snprintf(label, sizeof(label), "Control Point %d", i);
			const bool isSel = (rail->selectedControlPointIndex == i);
			if (isSel) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
			if (ImGui::Selectable(label, isSel)) rail->selectedControlPointIndex = i;
			if (isSel) ImGui::PopStyleColor();
		}
		ImGui::EndChild();

		const int sel = rail->selectedControlPointIndex;
		if (sel >= 0 && sel < static_cast<int>(rail->controlPoints.size())) {
			No::Vector3& p = rail->controlPoints[static_cast<size_t>(sel)];
			if (ImGui::DragFloat3("選択制御点位置", &p.x, 0.05f)) {
				rail->isLoaded = false; rail->needsRebuildArcLength = true;
			}
		}
	}
#else
	static_cast<void>(registry); static_cast<void>(railCameraEntity);
#endif
}
