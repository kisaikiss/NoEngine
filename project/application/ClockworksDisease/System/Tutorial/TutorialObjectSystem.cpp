#include "stdafx.h"
#include "TutorialObjectSystem.h"
#include "application/ClockworksDisease/Component/Tutorial/TutorialObjectComponent.h"
#include "application/ClockworksDisease/Component/Dialogue/DialogueComponent.h"


void TutorialObjectSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);

	bool isEventIssue = false;
	for (auto event : registry.PollAllEvents<DialogueEndEvent>()) {
		if (!registry.Has<TutorialSpeakerTag>(event.speaker)) {
			// チュートリアルスピーカータグを持っていなかったら他のシステムで利用する可能性があるため、取得したイベントをもう一度発行する
			registry.EmitEvent(event);
			continue;
		}

		isEventIssue = true;

	}

	// イベントが発行されていなければリターン
	if (!isEventIssue) return;

	// ルーチンコンポーネントを有効化する
	for (auto e : registry.View<TutorialMoverTag, No::TransformRoutineComponent>()) {
		registry.RemoveComponent<TutorialMoverTag>(e);
		registry.GetComponent<No::TransformRoutineComponent>(e)->playing = true;
	}

}

REGISTER_SYSTEM(::TutorialObjectSystem, "TutorialObjectSystem", "Tutorial")