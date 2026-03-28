#include "stdafx.h"
#include "GameEventEditor.h"

#include "application/CommentBout/Component/RailCameraComponent.h"
#include "application/CommentBout/GameTag.h"
#include <cstdio>

void GameEventEditor::DrawGameEventEditorImGui(No::Registry* registry, No::Entity railCameraEntity)
{
#ifdef USE_IMGUI
	if (!registry || railCameraEntity == No::nullEntity) {
		return;
	}

	auto* rail = registry->GetComponent<RailCameraComponent>(railCameraEntity);
	if (!rail) {
		return;
	}

	ImGui::Text("ステージイベント");
	if (ImGui::Button("敵生成イベント追加")) {
		RailEventData newEvent;
		newEvent.type = RailEventType::SpawnEnemy;
		newEvent.triggerDistance = rail->distance;
		rail->events.push_back(newEvent);
		rail->selectedEventIndex = static_cast<int>(rail->events.size()) - 1;
	}
	ImGui::SameLine();
	if (ImGui::Button("停止イベント追加")) {
		RailEventData newEvent;
		newEvent.type = RailEventType::RailStop;
		newEvent.triggerDistance = rail->distance;
		rail->events.push_back(newEvent);
		rail->selectedEventIndex = static_cast<int>(rail->events.size()) - 1;
	}
	ImGui::SameLine();
	if (ImGui::Button("再開イベント追加")) {
		RailEventData newEvent;
		newEvent.type = RailEventType::RailResume;
		newEvent.triggerDistance = rail->distance;
		newEvent.resumeCondition = RailResumeConditionType::AfterSeconds;
		newEvent.resumeAfterSeconds = 1.0f;
		rail->events.push_back(newEvent);
		rail->selectedEventIndex = static_cast<int>(rail->events.size()) - 1;
	}

	if (ImGui::Button("選択イベント削除")) {
		const int selected = rail->selectedEventIndex;
		if (selected >= 0 && selected < static_cast<int>(rail->events.size())) {
			rail->events.erase(rail->events.begin() + selected);
			if (rail->events.empty()) {
				rail->selectedEventIndex = -1;
			} else if (rail->selectedEventIndex >= static_cast<int>(rail->events.size())) {
				rail->selectedEventIndex = static_cast<int>(rail->events.size()) - 1;
			}
		}
	}

	for (int i = 0; i < static_cast<int>(rail->events.size()); ++i) {
		const RailEventData& e = rail->events[static_cast<size_t>(i)];
		char label[160];
		if (e.type == RailEventType::SpawnEnemy) {
			std::snprintf(label, sizeof(label), "イベント %d : 敵生成 [G%d] @ %.2f", i, e.spawn.spawnGroupId, e.triggerDistance);
		} else if (e.type == RailEventType::RailResume && e.resumeCondition == RailResumeConditionType::EnemiesCleared) {
			std::snprintf(label, sizeof(label), "イベント %d : 再開 [TargetG%d] @ %.2f", i, e.targetGroupId, e.triggerDistance);
		} else if (e.type == RailEventType::RailStop) {
			std::snprintf(label, sizeof(label), "イベント %d : 停止 @ %.2f", i, e.triggerDistance);
		} else {
			std::snprintf(label, sizeof(label), "イベント %d : 再開 @ %.2f", i, e.triggerDistance);
		}
		if (ImGui::Selectable(label, rail->selectedEventIndex == i)) {
			rail->selectedEventIndex = i;
		}
	}

	if (rail->selectedEventIndex >= 0 && rail->selectedEventIndex < static_cast<int>(rail->events.size())) {
		RailEventData& e = rail->events[static_cast<size_t>(rail->selectedEventIndex)];
		ImGui::Separator();
		int eventType = static_cast<int>(e.type);
		if (ImGui::Combo("イベント種別", &eventType, "敵生成\0レール停止\0レール再開\0")) {
			e.type = static_cast<RailEventType>(eventType);
		}
		ImGui::DragFloat("発生距離", &e.triggerDistance, 0.1f, 0.0f, (rail->totalLength > 0.0f) ? rail->totalLength : 10000.0f);

		if (e.type == RailEventType::SpawnEnemy) {
			int enemyType = static_cast<int>(e.spawn.enemyType);
			if (ImGui::Combo("敵種類", &enemyType, "MoveOnly\0MoveAndShoot\0Boss\0")) {
				e.spawn.enemyType = static_cast<RailEnemyType>(enemyType);
			}

			ImGui::DragInt("生成数", &e.spawn.count, 1.0f, 1, 32);
			// HPはEnemyConfigEditorSystemで管理するため、イベント側では編集しない。
			ImGui::DragFloat("敵速度", &e.spawn.moveSpeed, 0.1f, 0.0f, 100.0f);
			ImGui::DragFloat("生成間隔", &e.spawn.spawnSpacing, 0.1f, 0.0f, 20.0f);
			ImGui::DragFloat3("生成位置", &e.spawn.spawnPosition.x, 0.1f);
			ImGui::DragFloat3("移動方向", &e.spawn.moveDirection.x, 0.05f);
			ImGui::DragInt("生成グループID", &e.spawn.spawnGroupId, 1.0f, 0, 999);

			if (e.spawn.enemyType == RailEnemyType::Boss) {
				ImGui::Separator();
				ImGui::Text("Boss Params");
				ImGui::DragFloat3("Offset Local", &e.spawn.boss.offsetLocal.x, 0.05f);
				ImGui::DragFloat2("Figure8 Amplitude", &e.spawn.boss.figure8Amplitude.x, 0.05f, 0.0f, 20.0f);
				ImGui::DragFloat("Figure8 Period", &e.spawn.boss.figure8Period, 0.05f, 0.1f, 20.0f);
				ImGui::DragFloat("Stop Duration", &e.spawn.boss.stopDuration, 0.05f, 0.0f, 10.0f);
				ImGui::DragFloat("Burst Shot Interval", &e.spawn.boss.burstShotInterval, 0.01f, 0.05f, 2.0f);
			}
		}
		if (e.type == RailEventType::RailResume) {
			int conditionType = static_cast<int>(e.resumeCondition);
			if (ImGui::Combo("再開条件", &conditionType, "なし\0秒数\0敵全滅\0")) {
				e.resumeCondition = static_cast<RailResumeConditionType>(conditionType);
			}
			if (e.resumeCondition == RailResumeConditionType::AfterSeconds) {
				ImGui::DragFloat("再開秒数", &e.resumeAfterSeconds, 0.05f, 0.0f, 120.0f);
			}
			if (e.resumeCondition == RailResumeConditionType::EnemiesCleared) {
				ImGui::DragInt("対象グループID", &e.targetGroupId, 1.0f, 0, 999);
			}
		}

		ImGui::Text("実行状態: fired=%s waiting=%s elapsed=%.2f", e.fired ? "true" : "false", e.waitingCondition ? "true" : "false", e.waitingElapsedSeconds);
	}

#else
	static_cast<void>(registry);
	static_cast<void>(railCameraEntity);
#endif
}
