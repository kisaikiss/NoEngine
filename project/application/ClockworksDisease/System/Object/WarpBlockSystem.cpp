#include "stdafx.h"
#include "WarpBlockSystem.h"
#include "../../Component/Game/StageTransitionComponent.h"
#include "../Game/MainStageProgress.h"

void WarpBlockSystem::Update(No::Registry& registry, float deltaTime) {
	for (auto e : registry.View<StageTransitionComponent, No::TransformComponent>()) {
		auto* warpBlock = registry.GetComponent<StageTransitionComponent>(e);
		if (warpBlock->scalingTimer > 0.0f) {
			warpBlock->scalingTimer -= deltaTime;
			
			auto* transform = registry.GetComponent<No::TransformComponent>(e);

			// Easingに使用する
			const float easingTime = 1.0f - warpBlock->scalingTimer / warpBlock->transitionTime;
			transform->scale = No::Lerp(No::Vector3::UNIT_SCALE, No::Vector3::ZERO, No::ApplyEasing(No::EasingType::EaseInBack, easingTime));

			No::Vector3 warpBlockPos = warpBlock->collidePosition;
			warpBlockPos.y += std::sinf(easingTime * PI);
			transform->SetWorldPosition(registry, warpBlockPos);

			if (easingTime >= 1.0f) {
				if (warpBlock->destinationScene != No::GetCurrentSceneName(registry)) {
					MainStageProgress::QueueCapture();
					MainStageProgress::CaptureIfQueued(registry);
				}
				No::SceneChangeEvent sceneChangeEvent;
				sceneChangeEvent.nextScene = warpBlock->destinationScene;
				sceneChangeEvent.transitionType = No::SceneTransitionType::kCircleScale;
				registry.EmitEvent(sceneChangeEvent);
				transform->scale = No::Vector3::ZERO;
				warpBlock->scalingTimer = 0.0f;
			}

		}
	}
}

REGISTER_SYSTEM(::WarpBlockSystem, "WarpBlockSystem", "Gameplay")
