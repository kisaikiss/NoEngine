#include "PlayerBulletSystem.h"
#include <cmath>
#include "../GameTag.h"
#include "engine/Functions/Renderer/Primitive.h"

#ifdef USE_IMGUI
#include <Windows.h>
#include <string>
#include <sstream>
#endif

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
	auto view = registry.View<PlayerBulletComponent, PlayerBulletTag, DeathFlag, No::TransformComponent>();

	for (auto entity : view) {
		auto* bullet = registry.GetComponent<PlayerBulletComponent>(entity);
		auto* transform = registry.GetComponent<No::TransformComponent>(entity);
		auto* deathFlag = registry.GetComponent<DeathFlag>(entity);

		// ---- 移動 ----
		No::Vector3 movement = bullet->direction * bullet->speed * deltaTime;
		transform->translate = transform->translate + movement;
		bullet->travelDistance += bullet->speed * deltaTime;
		NoEngine::Primitive::DrawSphere(transform->translate, 1.0f, { 0.0f, 0.0f, 0.0f, 1.0f });
		
		// ---- 安全網：最大距離で消滅 ----
		if (bullet->travelDistance >= bullet->maxDistance) {
#ifdef USE_IMGUI
/*			std::ostringstream oss;
			oss << "[BULLET DESTROY] Entity:" << entity 
				<< " Reason:MaxDistance Travel:" << bullet->travelDistance 
				<< " Pos:(" << transform->translate.x << "," << transform->translate.y << ")\n";
			OutputDebugStringA(oss.str().c_str());
			*/

			#endif
			deathFlag->isDead = true;
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
#ifdef USE_IMGUI
			/*
			// フレーム毎の詳細ログ（最初の1秒間のみ）
			if (bullet->travelDistance < 5.0f) {
				std::ostringstream oss;
				oss << "[BULLET UPDATE] Entity:" << entity 
					<< " Travel:" << bullet->travelDistance 
					<< " Pos:(" << transform->translate.x << "," << transform->translate.y << ")"
					<< " NearNode:(" << nearestX << "," << nearestY << ")"
					<< " Dist:" << distFromNode << " SKIP:TooFar\n";
				OutputDebugStringA(oss.str().c_str());
			}*/
#endif
			continue;
		}

		// 発射元のノードは「発射直後の一瞬だけ」スキップする
		// NODE_DETECT_THRESHOLD * 2.0f（= 0.3f）未満の移動距離ならまだ始点ノードを出発したばかりと判断してスキップする。
		// これを超えたら始点ノードであっても通常通り前方接続チェックを行う。（行き止まりノードで発射した場合に素通りしてしまうバグへの対処）
		if (nearestX == bullet->startNodeX &&
			nearestY == bullet->startNodeY &&
			bullet->travelDistance < NODE_DETECT_THRESHOLD * 2.0f) {
#ifdef USE_IMGUI
			/*
			// フレーム毎の詳細ログ（最初の1秒間のみ）
			if (bullet->travelDistance < 5.0f) {
				std::ostringstream oss;
				oss << "[BULLET UPDATE] Entity:" << entity 
					<< " Travel:" << bullet->travelDistance 
					<< " Pos:(" << transform->translate.x << "," << transform->translate.y << ")"
					<< " NearNode:(" << nearestX << "," << nearestY << ")"
					<< " Dist:" << distFromNode << " SKIP:StartNode\n";
				OutputDebugStringA(oss.str().c_str());
			}*/
#endif
			continue;
		}

		// 前方接続チェック：接続がない、またはマップ外なら消滅
		if (ShouldDestroyAtNode(registry, nearestX, nearestY, bullet->direction)) {
#ifdef USE_IMGUI
			/*
			std::ostringstream oss;
			oss << "[BULLET DESTROY] Entity:" << entity 
				<< " Reason:NoConnection Travel:" << bullet->travelDistance 
				<< " Pos:(" << transform->translate.x << "," << transform->translate.y << ")"
				<< " Node:(" << nearestX << "," << nearestY << ")\n";
			OutputDebugStringA(oss.str().c_str());
			*/
#endif
			//削除
			deathFlag->isDead = true;
		} else {
#ifdef USE_IMGUI
			/*
			// フレーム毎の詳細ログ（最初の1秒間のみ）
			if (bullet->travelDistance < 5.0f) {
				std::ostringstream oss;
				oss << "[BULLET UPDATE] Entity:" << entity 
					<< " Travel:" << bullet->travelDistance 
					<< " Pos:(" << transform->translate.x << "," << transform->translate.y << ")"
					<< " NearNode:(" << nearestX << "," << nearestY << ")"
					<< " Dist:" << distFromNode << " CHECK:Pass\n";
				OutputDebugStringA(oss.str().c_str());
			}*/
#endif
		}

		
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