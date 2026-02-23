#include "PlayerBulletSystem.h"

void PlayerBulletSystem::Update(No::Registry& registry, float deltaTime) {
	auto view = registry.View<PlayerBulletComponent, No::TransformComponent>();

	std::vector<No::Entity> bulletsToRemove;

	for (auto entity : view) {
		auto* bullet = registry.GetComponent<PlayerBulletComponent>(entity);
		auto* transform = registry.GetComponent<No::TransformComponent>(entity);

		// 弾丸を移動
		No::Vector3 movement = bullet->direction * bullet->speed * deltaTime;
		transform->translate = transform->translate + movement;

		// 移動距離を記録
		bullet->travelDistance += bullet->speed * deltaTime;

		// 最大距離に到達したら削除
		if (bullet->travelDistance >= bullet->maxDistance) {
			bulletsToRemove.push_back(entity);
		}
	}

	// 削除リストの弾丸を削除
	for (auto entity : bulletsToRemove) {
		registry.DestroyEntity(entity);
	}
}
