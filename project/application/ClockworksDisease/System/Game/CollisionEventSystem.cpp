#include "CollisionEventSystem.h"
#include "CollisionLayer.h"
#include "CollisionEvents.h"
#include "../../Component/Player/PlayerComponent.h"
#include "../../Component/Game/StageTransitionComponent.h"
#include "MainStageProgress.h"

struct MagicScaffoldComponent{};

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

		// 接触した遷移オブジェクトに設定されたシーンへ移動する。
		No::Entity playerEntity = No::INVALID_ENTITY;
		No::Entity transitionEntity = No::INVALID_ENTITY;
		if (layerA && (layerA->layer & CollisionLayerComponent::Player) != CollisionLayerComponent::None &&
			registry.Has<StageTransitionComponent>(contact.b)) {
			playerEntity = contact.a;
			transitionEntity = contact.b;
		} else if (layerB && (layerB->layer & CollisionLayerComponent::Player) != CollisionLayerComponent::None &&
			registry.Has<StageTransitionComponent>(contact.a)) {
			playerEntity = contact.b;
			transitionEntity = contact.a;
		}
		if (playerEntity != No::INVALID_ENTITY) {
			const auto* transition = registry.GetComponent<StageTransitionComponent>(transitionEntity);
			if (transition && !transition->destinationScene.empty()) {
				if (No::GetCurrentSceneName(registry) == "GameScene" &&
					transition->destinationScene != "GameScene") {
					MainStageProgress::QueueCapture();
				}
				No::SceneChangeEvent event;
				event.nextScene = transition->destinationScene;
				event.transitionType = No::SceneTransitionType::kCircleScale;
				registry.EmitEvent(event);
			}
			continue;
		}
		if (!layerA || !layerB) continue;

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
		} else if ((layerA->layer & CollisionLayerComponent::Player) != CollisionLayerComponent::None &&
			(layerB->layer & CollisionLayerComponent::Item) != CollisionLayerComponent::None) {
			ItemGetEvent event;
			event.player = contact.a;
			event.item = contact.b;
			registry.EmitEvent(event);
		}




	}

}
