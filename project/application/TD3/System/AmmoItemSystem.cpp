#include "AmmoItemSystem.h"
#include "../GameTag.h"

void AmmoItemSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);// 未使用パラメータ警告回避

	// プレイヤーを取得
	auto playerView = registry.View<PlayerComponent, PlayerTag>();
	if (playerView.Empty()) return;

	PlayerComponent* player = nullptr;
	{
		auto it = playerView.begin();
		if (it != playerView.end()) {
			player = registry.GetComponent<PlayerComponent>(*it);
		}
	}

	if (!player) return;

	// 弾薬アイテムをチェック
	auto ammoView = registry.View<AmmoItemComponent, AmmoItemTag, DeathFlag>();
	if (ammoView.Empty()) {
		return;
	}

	for (No::Entity entity : ammoView) {
		auto* ammo = registry.GetComponent<AmmoItemComponent>(entity);
		auto* deathFlag = registry.GetComponent<DeathFlag>(entity);

		// プレイヤーと同じ座標かチェック
		if (player->currentNodeX == ammo->gridX &&
			player->currentNodeY == ammo->gridY &&
			ammo->canPickup) {
			
			// 弾数を回復（最大値を超えないように）
			player->currentBullets += ammo->ammoAmount;
			if (player->currentBullets > player->maxBullets) {
				player->currentBullets = player->maxBullets;
			}

			// アイテムを削除
			deathFlag->isDead = true;
		}
	}
}
