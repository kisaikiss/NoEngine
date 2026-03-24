#include "stdafx.h"
#include "RailCameraEditor.h"

#include "application/CommentBout/Component/RailCameraComponent.h"
#include "application/CommentBout/Component/EnemyComponent.h"
#include "application/CommentBout/GameTag.h"
#include <cstdio>

void RailCameraEditor::DrawRailCameraImGui(No::Registry* registry, No::Entity railCameraEntity, ResetEventRuntimeFn resetEventRuntime)
{
#ifdef USE_IMGUI
	if (!registry || railCameraEntity == No::nullEntity) {
		return;
	}

	auto* rail = registry->GetComponent<RailCameraComponent>(railCameraEntity);
	if (!rail) {
		return;
	}

	ImGui::Begin("レールカメラ");
	ImGui::Text("レールファイル: %s", rail->railFilePath.c_str());
	ImGui::Text("読込状態: %s", rail->isLoaded ? "読込済み" : "未読込");
	ImGui::Text("制御点数: %d", static_cast<int>(rail->controlPoints.size()));
	ImGui::Text("イベント数: %d", static_cast<int>(rail->events.size()));

	int aliveRailEnemyCount = 0;
	for (auto e : registry->View<CBRailEnemyTag, EnemyComponent>()) {
		(void)e;
		++aliveRailEnemyCount;
	}
	ImGui::Text("生存レール敵数: %d", aliveRailEnemyCount);

	if (rail->totalLength > 0.0f) {
		const float progress = rail->distance / rail->totalLength;
		ImGui::Text("進行率: %.1f%%", progress * 100.0f);
		ImGui::Text("距離: %.2f / %.2f", rail->distance, rail->totalLength);
	} else {
		ImGui::Text("進行率: 0.0%%");
		ImGui::Text("距離: %.2f / %.2f", rail->distance, rail->totalLength);
	}

	if (ImGui::SliderFloat("速度", &rail->speed, 0.0f, 40.0f) && rail->speed < 0.0f) {
		rail->speed = 0.0f;
	}
	ImGui::Checkbox("レール描画", &rail->drawRailDebug);
	ImGui::Checkbox("カメラギズモ描画", &rail->drawCameraDebug);
	ImGui::Checkbox("制御点描画", &rail->drawControlPointsDebug);
	ImGui::Checkbox("イベント点描画", &rail->drawEventPointsDebug);
	ImGui::DragFloat("制御点半径", &rail->controlPointDebugRadius, 0.005f, 0.01f, 1.0f);
	ImGui::DragFloat("イベント点半径", &rail->eventPointDebugRadius, 0.005f, 0.01f, 1.0f);

	if (ImGui::Button(rail->isPlaying ? "一時停止" : "再生")) {
		if (!rail->isFinished || rail->distance < rail->totalLength) {
			rail->isPlaying = !rail->isPlaying;
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("停止")) {
		rail->distance = 0.0f;
		rail->isPlaying = false;
		rail->isFinished = false;
	}
	ImGui::SameLine();
	if (ImGui::Button("先頭から再生")) {
		rail->distance = 0.0f;
		rail->isFinished = false;
		rail->isPlaying = true;
		if (resetEventRuntime) {
			resetEventRuntime(*rail);
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("イベント実行状態リセット") && resetEventRuntime) {
		resetEventRuntime(*rail);
	}

	float percent = 0.0f;
	if (rail->totalLength > 0.0f) {
		percent = (rail->distance / rail->totalLength) * 100.0f;
	}
	if (ImGui::SliderFloat("進行率(%)", &percent, 0.0f, 100.0f) && rail->totalLength > 0.0f) {
		rail->distance = rail->totalLength * (percent / 100.0f);
		rail->isFinished = (rail->distance >= rail->totalLength);
		if (rail->isFinished) {
			rail->isPlaying = false;
		}
	}

	if (rail->isFinished) {
		ImGui::Text("状態: 終了");
	} else if (rail->isPlaying) {
		ImGui::Text("状態: 再生中");
	} else {
		ImGui::Text("状態: 停止中");
	}

	ImGui::End();
#else
	static_cast<void>(registry);
	static_cast<void>(railCameraEntity);
	static_cast<void>(resetEventRuntime);
#endif
}

void RailCameraEditor::DrawRailEditorImGui(No::Registry* registry, No::Entity railCameraEntity)
{
#ifdef USE_IMGUI
	if (!registry || railCameraEntity == No::nullEntity) {
		return;
	}

	auto* rail = registry->GetComponent<RailCameraComponent>(railCameraEntity);
	if (!rail) {
		return;
	}

	ImGui::Begin("レール編集");

	if (ImGui::Button("制御点を追加")) {
		No::Vector3 newPoint = { 0.0f, 0.0f, 0.0f };
		if (!rail->controlPoints.empty()) {
			newPoint = rail->controlPoints.back();
			newPoint.z += 2.0f;
		}
		rail->controlPoints.push_back(newPoint);
		rail->selectedControlPointIndex = static_cast<int>(rail->controlPoints.size()) - 1;
		rail->isLoaded = false;
		rail->needsRebuildArcLength = true;
	}

	if (ImGui::Button("選択制御点を削除")) {
		const int index = rail->selectedControlPointIndex;
		if (index >= 0 && index < static_cast<int>(rail->controlPoints.size())) {
			rail->controlPoints.erase(rail->controlPoints.begin() + index);
			if (rail->controlPoints.empty()) {
				rail->selectedControlPointIndex = -1;
			} else if (rail->selectedControlPointIndex >= static_cast<int>(rail->controlPoints.size())) {
				rail->selectedControlPointIndex = static_cast<int>(rail->controlPoints.size()) - 1;
			}
			rail->isLoaded = false;
			rail->needsRebuildArcLength = true;
		}
	}

	const int selectedIndex = rail->selectedControlPointIndex;
	const bool canAddAfterSelected =
		(selectedIndex >= 0) &&
		(selectedIndex < static_cast<int>(rail->controlPoints.size()));
	if (!canAddAfterSelected) {
		ImGui::BeginDisabled();
	}
	if (ImGui::Button("選択点の次に追加")) {
		const int insertIndex = selectedIndex + 1;
		const No::Vector3& selectedPoint = rail->controlPoints[static_cast<size_t>(selectedIndex)];

		No::Vector3 direction = { 0.0f, 0.0f, 1.0f };
		if ((selectedIndex + 1) < static_cast<int>(rail->controlPoints.size())) {
			direction = rail->controlPoints[static_cast<size_t>(selectedIndex + 1)] - selectedPoint;
		} else if (selectedIndex > 0) {
			direction = selectedPoint - rail->controlPoints[static_cast<size_t>(selectedIndex - 1)];
		}

		if (direction.LengthSquared() <= 0.000001f) {
			direction = { 0.0f, 0.0f, 1.0f };
		} else {
			direction = direction.Normalize();
		}

		const float addDistance = 0.1f;
		const No::Vector3 newPoint = selectedPoint + direction * addDistance;
		rail->controlPoints.insert(rail->controlPoints.begin() + insertIndex, newPoint);
		rail->selectedControlPointIndex = insertIndex;
		rail->isLoaded = false;
		rail->needsRebuildArcLength = true;
	}
	if (!canAddAfterSelected) {
		ImGui::EndDisabled();
	}

	if (!rail->controlPoints.empty()) {
		ImGui::Separator();
		ImGui::Text("Control Points");
		ImGui::BeginChild("ControlPointList", ImVec2(0.0f, 220.0f), true);
		for (int i = 0; i < static_cast<int>(rail->controlPoints.size()); ++i) {
			char label[48];
			std::snprintf(label, sizeof(label), "Control Point %d", i);
			const bool isSelected = (rail->selectedControlPointIndex == i);
			if (isSelected) {
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
			}
			if (ImGui::Selectable(label, isSelected)) {
				rail->selectedControlPointIndex = i;
			}
			if (isSelected) {
				ImGui::PopStyleColor();
			}
		}
		ImGui::EndChild();

		const int selected = rail->selectedControlPointIndex;
		if (selected >= 0 && selected < static_cast<int>(rail->controlPoints.size())) {
			No::Vector3& p = rail->controlPoints[static_cast<size_t>(selected)];
			if (ImGui::DragFloat3("選択制御点位置", &p.x, 0.05f)) {
				rail->isLoaded = false;
				rail->needsRebuildArcLength = true;
			}
		}
	}

	ImGui::End();
#else
	static_cast<void>(registry);
	static_cast<void>(railCameraEntity);
#endif
}
