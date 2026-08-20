#include "stdafx.h"
#include "ItemGetSystem.h"
#include "../Game/CollisionEvents.h"
#include "../../Component/Item/ItemComponent.h"
#include "../../Component/Player/PlayerComponent.h"
#include "../../Component/Game/GameProgressComponent.h"
#include "../../Component/Game/GoalDirectionComponent.h"
#include "engine/Editor/DataDriven/SceneSerializer.h"

namespace {

void IncrementCollectedCount(No::Registry& registry, No::Entity item) {
	if (!registry.Has<CollectibleItemTag>(item)) return;
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
			registry.AddComponent<No::EffectEmitTag>(event.item);
			registry.DestroyEntity(event.item);
			constexpr uint32_t kBadgePower = 1;
			registry.GetComponent<LevelComponent>(event.player)->power += kBadgePower;
			continue;
		}

		if (registry.Has<BigPowerItemComponent>(event.item)) {
			IncrementCollectedCount(registry, event.item);
			registry.AddComponent<BigPowerGetTag>(event.item);
			registry.GetComponent<No::ParticleEmitterComponent>(event.item)->active = true;
			registry.RemoveComponent<No::SphereCollider>(event.item);

			registry.GetComponent<LevelComponent>(event.player)->power += registry.GetComponent<BigPowerItemComponent>(event.item)->grantPower;
		}
	}
}