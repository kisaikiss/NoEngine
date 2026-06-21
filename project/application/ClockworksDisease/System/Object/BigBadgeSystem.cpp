#include "stdafx.h"
#include "BigBadgeSystem.h"
#include "application/ClockworksDisease/Component/Item/ItemComponent.h"
#include "application/ClockworksDisease/Component/Player/PlayerComponent.h"

void BigBadgeSystem::Update(No::Registry& registry, float deltaTime) {
	for (auto e : registry.View<BigBadgeComponent, BigBadgeGetTag, No::TransformComponent>()) {
		auto* badge = registry.GetComponent<BigBadgeComponent>(e);
		
		auto* transform = registry.GetComponent<No::TransformComponent>(e);
		No::Vector3 playerWorldPos = No::Vector3::ZERO;
		for (auto playerEntity : registry.View< PlayerComponent, No::TransformComponent>()) {
			playerWorldPos = registry.GetComponent<No::TransformComponent>(playerEntity)->GetWorldPosition();
		}


		const float kThetaDeltaTimeMagnification = 7.f;
		badge->theta += deltaTime * kThetaDeltaTimeMagnification;
		if (badge->t < 1.f) {
			const float kTDeltaTimeMagnification = 0.5f;
			badge->t += deltaTime * kTDeltaTimeMagnification;

			const float kStartTranslateMagnification = 6.f;
			badge->translateMagnification = std::lerp(kStartTranslateMagnification, 0.0f, badge->t);
		} else {
			registry.GetComponent<No::AnimatorComponent>(e)->animationSpeedMagnification = 50.f;
			const float kScaleThetaDeltaTimeMagnification = 2.f;
			badge->scaleT += deltaTime * kScaleThetaDeltaTimeMagnification;
			transform->scale = No::EaseInBack(No::Vector3(4.f, 4.f, 4.f), No::Vector3::ZERO, badge->scaleT);
			if (badge->scaleT > 1.f) {
				registry.AddComponent<No::EffectEmitTag>(e);
				registry.DestroyEntity(e);
			}
		}

		const float kFinalPositionOffset = 5.5f;
		const float kPositionOffset = 1.f;
		badge->yPositionOffset = No::EaseInExpo(0.f, kFinalPositionOffset, badge->t);

		transform->translate.x = playerWorldPos.x + std::sinf(badge->theta) * badge->translateMagnification;
		transform->translate.y = playerWorldPos.y + badge->yPositionOffset + kPositionOffset;
		transform->translate.z = playerWorldPos.z + std::cosf(badge->theta) * badge->translateMagnification;
	}
}
