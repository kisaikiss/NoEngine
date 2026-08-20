#include "stdafx.h"
#include "GoalDirectionSystem.h"
#include "../../Component/Game/GoalDirectionComponent.h"
#include "../../Component/Game/GameProgressComponent.h"
#include "GameResult.h"

void GoalDirectionSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);

	for (auto e : registry.View<GoalDirectionComponent>()) {
		auto* dir = registry.GetComponent<GoalDirectionComponent>(e);

		// カメラ側・ゴール側それぞれのTransformRoutineComponentが再生完了(playing=false)するまで待つ。
		// TransformRoutineComponentを持っていない側は待たない扱いにする。
		bool cameraDone = true;
		if (auto* camRoutine = registry.GetComponent<No::TransformRoutineComponent>(dir->directorCamera)) {
			cameraDone = !camRoutine->playing;
		}
		bool goalDone = true;
		if (auto* goalRoutine = registry.GetComponent<No::TransformRoutineComponent>(dir->goalEntity)) {
			goalDone = !goalRoutine->playing;
		}

		if (!cameraDone || !goalDone) continue;

		// 結果をGameClearSceneへ橋渡しする
		GameResult::Data result;
		for (auto progressE : registry.View<GameProgressComponent>()) {
			auto* progress = registry.GetComponent<GameProgressComponent>(progressE);
			result.totalItemCount = progress->totalItemCount;
			result.collectedItemCount = progress->collectedItemCount;
			result.clearTime = progress->elapsedTime;
		}
		GameResult::Set(result);

		// GameClearSceneへ遷移する
		No::SceneChangeEvent sceneChangeEvent;
		sceneChangeEvent.nextScene = "GameClearScene";
		registry.EmitEvent(sceneChangeEvent);

		// 役目を終えたのでロックと管理Entityを片付ける
		if (dir->player != No::INVALID_ENTITY) {
			registry.RemoveComponent<GoalDirectionLockTag>(dir->player);
		}
		registry.DestroyEntity(e);
	}
}