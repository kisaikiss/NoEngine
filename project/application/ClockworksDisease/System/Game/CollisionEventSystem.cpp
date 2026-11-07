#include "CollisionEventSystem.h"
#include "CollisionLayer.h"
#include "CollisionEvents.h"
#include "../../Component/Enemy/WanderHopEnemyComponent.h"
#include "../../Component/Player/PlayerComponent.h"
#include "../../Component/Game/StageTransitionComponent.h"
#include "../Object/MagicScaffoldSystem.h"
#include "MainStageProgress.h"


void CollisionEventSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	// 衝突イベントを取り出す
	auto contactEvent = registry.PollEvent<No::ContactEvent>();
	if (!contactEvent.has_value()) return;
	for (auto contact : contactEvent->contacts) {
		// ここでゲームアプリケーション用の衝突イベントを発行する
		// 例えば、プレイヤーと地面の接触イベントを発行するなど
		// プレイヤーと地面の接触イベントは、プレイヤーが接地しているかどうかを管理するために使用される
		auto* layerA = registry.GetComponent<CollisionLayerComponent>(contact.a);
		auto* layerB = registry.GetComponent<CollisionLayerComponent>(contact.b);

		if (!layerA || !layerB) continue;

		const auto actorMask = CollisionLayerComponent::Player | CollisionLayerComponent::Enemy;
		// 本体接触ではプレイヤー・敵をa側に揃え、法線と接触方向も反転する。
		if (!contact.isSensor && (layerA->layer & actorMask) == CollisionLayerComponent::None
			&& (layerB->layer & actorMask) != CollisionLayerComponent::None) {
			std::swap(contact.a, contact.b);
			std::swap(layerA, layerB);
			contact.normal = -contact.normal;
			if (contact.contactPosition == No::ContactPosition::UP) contact.contactPosition = No::ContactPosition::DOWN;
			else if (contact.contactPosition == No::ContactPosition::DOWN) contact.contactPosition = No::ContactPosition::UP;
		}
		const bool playerA = (layerA->layer & CollisionLayerComponent::Player) != CollisionLayerComponent::None;
		const bool playerB = (layerB->layer & CollisionLayerComponent::Player) != CollisionLayerComponent::None;
		const bool enemyA = (layerA->layer & CollisionLayerComponent::Enemy) != CollisionLayerComponent::None;
		const bool enemyB = (layerB->layer & CollisionLayerComponent::Enemy) != CollisionLayerComponent::None;
		if (contact.isSensor) {
			if (playerA && enemyB) registry.EmitEvent(PlayerAttackHitEvent{contact.a, contact.b});
			continue;
		}
		if (playerA && enemyB) {
			registry.EmitEvent(PlayerEnemyContactEvent{contact.a, contact.b});
		} else if (enemyA && playerB) {
			registry.EmitEvent(PlayerEnemyContactEvent{contact.b, contact.a});
		}


		if ((layerA->layer & CollisionLayerComponent::Player) != CollisionLayerComponent::None &&
			(layerB->layer & CollisionLayerComponent::Terrain) != CollisionLayerComponent::None) {
			PlayerPushBackEvent event;
			event.player = contact.a;
			event.platform = contact.b;
			event.position = contact.contactPosition;
			event.normal = contact.normal;
			event.penetration = contact.penetration;
			// 魔法足場以外の足場にのった時は足場生成可能フラグをtrueにする
			if (!registry.Has<MagicScaffoldComponent>(contact.b)&& contact.contactPosition == No::ContactPosition::UP) {
				registry.GetComponent<PlayerComponent>(contact.a)->canCreateScaffold = true;
			}
			registry.EmitEvent(event);
		} else if ((layerA->layer & CollisionLayerComponent::Enemy) != CollisionLayerComponent::None &&
			(layerB->layer & CollisionLayerComponent::Terrain) != CollisionLayerComponent::None) {
			auto* ground = registry.GetComponent<No::GroundStateComponent>(contact.a);
			auto* velocity = registry.GetComponent<No::VelocityComponent>(contact.a);
			auto* enemy = registry.GetComponent<WanderHopEnemyStateComponent>(contact.a);
			if (!ground || !velocity) continue;
			const bool hoppingUp = enemy && enemy->phase == WanderHopEnemyStateComponent::Phase::AlertHop &&
				(enemy->justJumped || velocity->linear.y > 0.0f);
			if (!hoppingUp && contact.contactPosition == No::ContactPosition::UP &&
				contact.normal.y >= ground->snapMinNormalY) {
				ground->isGrounded = true;
				ground->groundHeight = registry.GetComponent<No::TransformComponent>(contact.a)->GetWorldPosition(registry).y;
				ground->groundEntity = contact.b;
			}
		} else if ((layerA->layer & CollisionLayerComponent::Player) != CollisionLayerComponent::None &&
			(layerB->layer & CollisionLayerComponent::Item) != CollisionLayerComponent::None) {
			ItemGetEvent event;
			event.player = contact.a;
			event.item = contact.b;
			registry.EmitEvent(event);
		}




	}

}

REGISTER_SYSTEM(::CollisionEventSystem, "CollisionEventSystem", "Gameplay")
