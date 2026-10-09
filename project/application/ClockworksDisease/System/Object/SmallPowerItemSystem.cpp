#include "stdafx.h"
#include "SmallPowerItemSystem.h"
#include "../../Component/Item/ItemComponent.h"
#include "../../Component/Player/PlayerComponent.h"

void SmallPowerItemSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);

	for (auto e : registry.View<PowerItemComponent, PowerItemGetTag, No::TransformComponent>()) {


		auto* item = registry.GetComponent<PowerItemComponent>(e);
		item->moveTimer += deltaTime;
		if (item->risen) {
			No::Entity playerEntity = registry.GetComponent<PowerItemGetTag>(e)->playerEntity;
			No::Vector3 playerPos = registry.GetComponent<No::TransformComponent>(playerEntity)->GetWorldPosition(registry);

			constexpr float playerPosOffset = 1.0f;
			playerPos.y += playerPosOffset;
			auto* transform = registry.GetComponent<No::TransformComponent>(e);
			transform->SetWorldPosition(registry, No::Lerp(transform->GetWorldPosition(registry) , playerPos, No::ApplyEasing(No::EasingType::Linear,item->moveTimer)));
			transform->scale = No::Lerp(transform->scale, No::Vector3::ZERO, No::ApplyEasing(No::EasingType::Linear, item->moveTimer));

			if (item->moveTimer > item->attractTime) {
				// エフェクトを出す位置をプレイヤーの座標に合わせる
				transform->SetWorldPosition(registry,playerPos);
				registry.AddComponent<No::EffectEmitTag>(e);
				registry.DestroyEntity(e);
			}

		} else {
			if (item->moveTimer > item->riseTime) {
				registry.RemoveComponent<No::VelocityComponent>(e);
				item->risen = true;
				item->moveTimer = 0.0f;
			}
		}

	}

}

#include "engine/Functions/ECS/System/SystemManager.h"
REGISTER_SYSTEM(::SmallPowerItemSystem, "SmallPowerItemSystem", "Gameplay")
