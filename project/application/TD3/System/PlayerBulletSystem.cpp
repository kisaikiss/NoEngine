#include "PlayerBulletSystem.h"
#include <cmath>
#include <vector>

// ============================================================
// 定数定義
// ============================================================

// 現在位置がノード座標付近と判定する距離の閾値
// グリッド間隔が 1.0 のため 0.15f で十分に検出できるはず？。
// 弾速 5.0f ・ 60fps 時の 1 フレームの移動量は約 0.083f なので、ノードを通過しても必ず 1〜2 フレーム以内に検出される。
static constexpr float NODE_DETECT_THRESHOLD = 0.15f;

// ============================================================
//  Update
// ============================================================

void PlayerBulletSystem::Update(No::Registry& registry, float deltaTime) {
	auto view = registry.View<PlayerBulletComponent, No::TransformComponent>();

	std::vector<No::Entity> bulletsToRemove;

	for (auto entity : view) {
		auto* bullet = registry.GetComponent<PlayerBulletComponent>(entity);
		auto* transform = registry.GetComponent<No::TransformComponent>(entity);

		// ---- 移動 ----
		No::Vector3 movement = bullet->direction * bullet->speed * deltaTime;
		transform->translate = transform->translate + movement;
		bullet->travelDistance += bullet->speed * deltaTime;

		// ---- 安全網：最大距離で消滅 ----
		// グリッド判定をすり抜けた弾が永遠に飛び続けないようにする保険
		if (bullet->travelDistance >= bullet->maxDistance) {
			bulletsToRemove.push_back(entity);
			continue;
		}

		// ---- グリッドベース消滅判定 ----
		//始点ノードのスキップを「距離ガード（MIN_TRAVEL_FOR_CHECK）」で行うと、エッジ途中から発射した場合に次のノードまでの距離が 0.5f 未満になり、そのノードの判定を丸ごとスキップしてしまうバグが発生する。
		//そのため始点スキップは「startNodeX/Y と一致するノードのみ」という座標ベースの判定で行う。これにより距離に依存せず確実にスキップできる。

		// 現在位置に最も近い整数座標（グリッドノード候補）を求める
		int nearestX = static_cast<int>(std::round(transform->translate.x));
		int nearestY = static_cast<int>(std::round(transform->translate.y));

		// そのノードまでの距離を計算する
		float dx = transform->translate.x - static_cast<float>(nearestX);
		float dy = transform->translate.y - static_cast<float>(nearestY);
		float distFromNode = std::sqrt(dx * dx + dy * dy);

		// ノード付近にいる場合のみ判定する
		if (distFromNode >= NODE_DETECT_THRESHOLD) {
			continue;
		}

		// 発射元のノードは「発射直後の一瞬だけ」スキップする
		// NODE_DETECT_THRESHOLD * 2.0f（= 0.3f）未満の移動距離ならまだ始点ノードを出発したばかりと判断してスキップする。
		// これを超えたら始点ノードであっても通常通り前方接続チェックを行う。（行き止まりノードで発射した場合に素通りしてしまうバグへの対処）
		if (nearestX == bullet->startNodeX &&
			nearestY == bullet->startNodeY &&
			bullet->travelDistance < NODE_DETECT_THRESHOLD * 2.0f) {
			continue;
		}

		// 前方接続チェック：接続がない、またはマップ外なら消滅
		if (ShouldDestroyAtNode(registry, nearestX, nearestY, bullet->direction)) {
			bulletsToRemove.push_back(entity);
		}
	}

	// 削除リストの弾丸を削除
	for (auto entity : bulletsToRemove) {
		registry.DestroyEntity(entity);
	}
}

// ============================================================
//  ShouldDestroyAtNode
// ============================================================

bool PlayerBulletSystem::ShouldDestroyAtNode(
	No::Registry& registry,
	int nodeX, int nodeY,
	const No::Vector3& direction
) {
	auto* cell = GetGridCell(registry, nodeX, nodeY);

	// グリッドセルが存在しない（マップ外）のとき消滅
	if (!cell) {
		return true;
	}

	// 進行方向ベクトルからグリッド上の前方接続を判定する
	// 弾は軸平行（上下左右）にしか飛ばないため、最大成分の符号で方向を決定する
	bool hasForwardConnection = false;

	if (direction.y > 0.5f) {
		// Up（Y+ 方向）
		hasForwardConnection = cell->hasConnectionUp;
	} else if (direction.y < -0.5f) {
		// Down（Y- 方向）
		hasForwardConnection = cell->hasConnectionDown;
	} else if (direction.x > 0.5f) {
		// Right（X+ 方向）
		hasForwardConnection = cell->hasConnectionRight;
	} else if (direction.x < -0.5f) {
		// Left（X- 方向）
		hasForwardConnection = cell->hasConnectionLeft;
	}

	// 前方への接続がない場合消滅
	return !hasForwardConnection;
}

// ============================================================
//  GetGridCell
// ============================================================

GridCellComponent* PlayerBulletSystem::GetGridCell(
	No::Registry& registry,
	int x, int y
) {
	auto view = registry.View<GridCellComponent>();
	for (auto entity : view) {
		auto* cell = registry.GetComponent<GridCellComponent>(entity);
		if (cell->gridX == x && cell->gridY == y) {
			return cell;
		}
	}
	return nullptr;
}