#include "stdafx.h"
#include "RabbitdokuCollisionEventSystem.h"
#include "../Game/RabbitdokuCollisionLayer.h"

void RabbitdokuCollisionEventSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	// 衝突イベントを取り出す
	auto contactEvent = registry.PollEvent<No::Contact2DEvent>();
	if (!contactEvent.has_value()) return;
	for (const auto& contact : contactEvent->contacts) {
		// ここでゲームアプリケーション用の衝突イベントを発行する
		// 例えば、プレイヤーと地面の接触イベントを発行するなど
		// プレイヤーと地面の接触イベントは、プレイヤーが接地しているかどうかを管理するために使用される
		auto* layerA = registry.GetComponent<RabbitdokuCollisionLayerComponent>(contact.a);
		auto* layerB = registry.GetComponent<RabbitdokuCollisionLayerComponent>(contact.b);
		if (!layerA || !layerB) continue;

		if ((layerA->layer & RabbitdokuCollisionLayerComponent::Player) != RabbitdokuCollisionLayerComponent::None &&
			(layerB->layer & RabbitdokuCollisionLayerComponent::Terrain) != RabbitdokuCollisionLayerComponent::None) {
			RabbitdokuPushBackEvent event;
			event.player = contact.a;
			event.position = contact.contactPosition;
			registry.EmitEvent(event);
		}

		if ((layerA->layer & RabbitdokuCollisionLayerComponent::Player) != RabbitdokuCollisionLayerComponent::None &&
			(layerB->layer & RabbitdokuCollisionLayerComponent::Item) != RabbitdokuCollisionLayerComponent::None) {
			RabbitdokuItemGetEvent event;
			event.player = contact.a;
			event.item = contact.b;
			registry.EmitEvent(event);
		}

	}
}
