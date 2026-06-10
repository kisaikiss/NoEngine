#include "CollisionEventSystem.h"
#include "CollisionLayer.h"
#include "CollisionEvents.h"

void CollisionEventSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	// 衝突イベントを取り出す
	auto contactEvent = registry.PollEvent<No::ContactEvent>();
	if (!contactEvent.has_value()) return;
	for (const auto& contact : contactEvent->contacts) {
		// ここでゲームアプリケーション用の衝突イベントを発行する
		// 例えば、プレイヤーと地面の接触イベントを発行するなど
		// プレイヤーと地面の接触イベントは、プレイヤーが接地しているかどうかを管理するために使用される
		auto* layerA = registry.GetComponent<CollisionLayerComponent>(contact.a);
		auto* layerB = registry.GetComponent<CollisionLayerComponent>(contact.b);
		if (!layerA || !layerB) continue;

		if ((layerA->layer & CollisionLayerComponent::Player) != CollisionLayerComponent::None &&
			(layerB->layer & CollisionLayerComponent::Terrain) != CollisionLayerComponent::None) {
			PlayerPushBackEvent event;
			event.player = contact.a;
			event.position = contact.contactPosition;
			event.normal = contact.normal;
			registry.EmitEvent(event);
		}


	}

}
