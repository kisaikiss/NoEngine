#include "stdafx.h"
#include "SceneChangeTimerSystem.h"

REFLECT_STRUCT_BEGIN(SceneChangeTimerComponent, "Opening")
REFLECT_FIELD(sceneChangeTime)
REFLECT_STRUCT_END(SceneChangeTimerComponent)

void SceneChangeTimerSystem::Update(No::Registry& registry, float deltaTime) {
	sceneChangeTimer_ += deltaTime;
	for (auto e : registry.View<SceneChangeTimerComponent>()) {
		auto* changeTime = registry.GetComponent<SceneChangeTimerComponent>(e);

		// 設定した時間が過ぎたらシーンを変える
		if (sceneChangeTimer_ >= changeTime->sceneChangeTime) {
			No::SceneChangeEvent sceneChangeEvent;
			sceneChangeEvent.nextScene = "GameScene";
			registry.EmitEvent(sceneChangeEvent);

			// タイマーコンポーネントは不要になるので外す
			registry.RemoveComponent<SceneChangeTimerComponent>(e);
		}

	}

}
