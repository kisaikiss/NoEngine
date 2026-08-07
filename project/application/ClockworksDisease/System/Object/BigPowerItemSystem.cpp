#include "stdafx.h"
#include "BigPowerItemSystem.h"
#include "application/ClockworksDisease/Component/Item/ItemComponent.h"
#include "application/ClockworksDisease/Component/Player/PlayerComponent.h"

void BigPowerItemSystem::Update(No::Registry& registry, float deltaTime) {
	for (auto e : registry.View<BigPowerItemComponent, BigPowerGetTag, No::TransformComponent>()) {
		auto* badge = registry.GetComponent<BigPowerItemComponent>(e);
		
		auto* transform = registry.GetComponent<No::TransformComponent>(e);
		No::Vector3 playerWorldPos = No::Vector3::ZERO;
		for (auto playerEntity : registry.View< PlayerComponent, No::TransformComponent>()) {
			playerWorldPos = registry.GetComponent<No::TransformComponent>(playerEntity)->GetWorldPosition(registry);
		}


		constexpr float kThetaDeltaTimeMagnification = 7.f;
		badge->theta += deltaTime * kThetaDeltaTimeMagnification;
		if (badge->t < 1.f) {
			constexpr float kTDeltaTimeMagnification = 0.5f;
			badge->t += deltaTime * kTDeltaTimeMagnification;

			constexpr float kStartTranslateMagnification = 6.f;
			badge->translateMagnification = std::lerp(kStartTranslateMagnification, 0.0f, badge->t);
		} else {
			constexpr float kScaleThetaDeltaTimeMagnification = 2.f;
			badge->scaleT += deltaTime * kScaleThetaDeltaTimeMagnification;
			transform->scale = No::Lerp(No::Vector3(2.f, 2.f, 2.f), No::Vector3::ZERO, No::ApplyEasing(No::EasingType::EaseInBack, badge->scaleT));
			if (badge->scaleT > 1.f) {
				registry.AddComponent<No::EffectEmitTag>(e);
				registry.DestroyEntity(e);
			}
		}

		constexpr float kFinalPositionOffset = 2.0f;
		constexpr float kPositionOffset = 1.f;
		badge->yPositionOffset = No::Lerp(0.f, kFinalPositionOffset,No::ApplyEasing(No::EasingType::EaseInExpo, badge->t));

		transform->translate.x = playerWorldPos.x + std::sinf(badge->theta) * badge->translateMagnification;
		transform->translate.y = playerWorldPos.y + badge->yPositionOffset + kPositionOffset;
		transform->translate.z = playerWorldPos.z + std::cosf(badge->theta) * badge->translateMagnification;
	}
}
