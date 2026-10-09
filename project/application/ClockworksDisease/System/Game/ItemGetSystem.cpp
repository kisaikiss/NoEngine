#include "stdafx.h"
#include "ItemGetSystem.h"
#include "../Game/CollisionEvents.h"
#include "../../Component/Item/ItemComponent.h"
#include "../../Component/Player/PlayerComponent.h"
#include "../../Component/Game/GameProgressComponent.h"
#include "../../Component/Game/GoalDirectionComponent.h"
#include "../../Component/Game/StageTransitionComponent.h"
#include "application/ClockworksDisease/Component/Camera/FollowCameraComponent.h"
#include "application/ClockworksDisease/Component/Camera/CameraIntroComponent.h"
#include "MainStageProgress.h"

namespace {

void IncrementCollectedCount(No::Registry& registry, No::Entity item) {
	if (!registry.Has<CollectibleItemTag>(item)) return;
	MainStageProgress::RecordCollectedItem(registry, item);
	for (auto e : registry.View<GameProgressComponent>()) {
		registry.GetComponent<GameProgressComponent>(e)->collectedItemCount++;
	}
}

bool GoalDirectionInProgress(No::Registry& registry) {
	auto view = registry.View<GoalDirectionComponent>();
	return view.begin() != view.end();
}

// ゴール接触時、即座にシーン遷移せず演出用のGoalDirectionComponentを生成して開始する。
// 演出用カメラはシーンにあらかじめ配置しておき、GoalDirectorCameraTagで検索する。
void StartGoalDirection(No::Registry& registry, No::Entity player, No::Entity goalItem) {

	No::Entity directorCamera = No::INVALID_ENTITY;
	for (auto e : registry.View<GoalDirectorCameraTag>()) {
		directorCamera = e;
	}

	if (directorCamera == No::INVALID_ENTITY) return;

	auto director = registry.GenerateEntity();
	auto* dir = registry.AddComponent<GoalDirectionComponent>(director);
	dir->directorCamera = directorCamera;
	dir->goalEntity = goalItem;
	dir->player = player;

	// 演出用カメラへ主導権を渡す
	if (directorCamera != No::INVALID_ENTITY) {
		registry.AddComponent<No::ActiveCameraTag>(directorCamera);

		// 再利用に備え、演出パスを最初から再生する
		if (auto* camRoutine = registry.GetComponent<No::TransformRoutineComponent>(directorCamera)) {
			camRoutine->currentIndex = 0;
			camRoutine->elapsed = 0.0f;
			camRoutine->playing = true;
		}
	}

	// ゴールオブジェクト側の演出パスも同様に再生開始する(持っていなければ何もしない)
	if (auto* goalRoutine = registry.GetComponent<No::TransformRoutineComponent>(goalItem)) {
		goalRoutine->currentIndex = 0;
		goalRoutine->elapsed = 0.0f;
		goalRoutine->playing = true;
	}

	// プレイヤーの移動/ジャンプ/重力系Systemを止める
	registry.AddComponent<GoalDirectionLockTag>(player);
	if (auto* velocity = registry.GetComponent<No::VelocityComponent>(player)) {
		velocity->linear = No::Vector3::ZERO;
	}
}

} // namespace

void ItemGetSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);

	auto events = registry.PollAllEvents<ItemGetEvent>();
	for (auto event : events) {
		// ゴールアイテムなら他の処理より先に演出を開始する(即座には遷移しない)
		if (registry.Has<GoalItemTag>(event.item)) {
			if (!GoalDirectionInProgress(registry)) {
				StartGoalDirection(registry, event.player, event.item);
			}
			continue;
		}

		if (registry.Has<PowerItemComponent>(event.item)) {
			IncrementCollectedCount(registry, event.item);
			registry.AddComponent<PowerItemGetTag>(event.item)->playerEntity = event.player;
			registry.AddComponent<No::VelocityComponent>(event.item)->linear.y = registry.GetComponent<PowerItemComponent>(event.item)->riseSpeed;
			registry.RemoveComponent<No::SphereCollider>(event.item);
			constexpr uint32_t kPower = 1;
			registry.GetComponent<LevelComponent>(event.player)->power += kPower;
			continue;
		}

		if (registry.Has<BigPowerItemComponent>(event.item)) {
			IncrementCollectedCount(registry, event.item);
			registry.AddComponent<BigPowerGetTag>(event.item);
			registry.GetComponent<No::ParticleEmitterComponent>(event.item)->active = true;
			registry.RemoveComponent<No::SphereCollider>(event.item);

			registry.GetComponent<LevelComponent>(event.player)->power += registry.GetComponent<BigPowerItemComponent>(event.item)->grantPower;
		}

		if (registry.Has<SavePointComponent>(event.item)) {
			auto* savePoint = registry.GetComponent<SavePointComponent>(event.item);
			auto* player = registry.GetComponent<PlayerComponent>(event.player);
			auto* transform = registry.GetComponent<No::TransformComponent>(event.item);
			if (player->respawnPoint == transform->GetWorldPosition(registry)) continue;

			savePoint->rotateTimer = savePoint->rotateTime;
			registry.GetComponent<No::AnimatorComponent>(event.item)->animationSpeedMagnification = 20.0f;
			registry.GetComponent<No::MaterialComponent>(event.item)->color = No::Color::YELLOW * savePoint->colorMagnification;

			player->respawnPoint = transform->GetWorldPosition(registry);
		}

		if (registry.HasAll<StageTransitionComponent, No::TransformComponent, No::SphereCollider>(event.item)) {
			auto* transition = registry.GetComponent<StageTransitionComponent>(event.item);
			registry.RemoveComponent<No::SphereCollider>(event.item);

			if (!transition->destinationScene.empty()) {
				registry.GetComponent<No::MeshComponent>(event.player)->isVisible = false;
				registry.GetComponent<No::MaterialComponent>(event.player)->castShadow = false;
				registry.GetComponent<No::VelocityComponent>(event.player)->linear = No::Vector3::ZERO;
				if (registry.Has<No::ParticleEmitterSphereComponent>(event.player))
					registry.GetComponent<No::ParticleEmitterSphereComponent>(event.player)->active = false;
				for (auto e : registry.View<FollowCameraComponent>()) {
					registry.AddComponent<CameraLockTag>(e);
				}


				transition->scalingTimer = transition->transitionTime;
				transition->collidePosition = registry.GetComponent<No::TransformComponent>(event.item)->GetWorldPosition(registry);
			}
			continue;
		}
	}
}

#include "engine/Functions/ECS/System/SystemManager.h"
REGISTER_SYSTEM(::ItemGetSystem, "ItemGetSystem", "Gameplay")
