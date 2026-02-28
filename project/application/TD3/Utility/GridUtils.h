#pragma once
#include "engine/NoEngine.h"
#include "../Component/GridCellComponent.h"
#include "../Component/PlayerComponent.h" // Direction enum

/// <summary>
/// グリッド関連の共通ユーティリティ
/// 重複していたコードを一元管理する。
/// すべて inline 関数として定義し、ヘッダーで使用
/// </summary>
namespace GridUtils {

	// ============================================================
	//  グローバル設定
	// ============================================================

	/// <summary>
	/// グリッド1マスのワールド単位スケール。
	/// エディタやJSONから変更可能。デフォルトは1.0f。
	/// </summary>
	inline float gGridScale = 1.0f;

	// ============================================================
	//  座標変換
	// ============================================================

	/// <summary>
	/// グリッド座標をワールド座標に変換する
	/// 通常はこちらを使用する。
	/// </summary>
	/// <param name="x">グリッドX座標</param>
	/// <param name="y">グリッドY座標</param>
	inline No::Vector3 GridToWorld(int x, int y) {
		return No::Vector3{
			static_cast<float>(x) * gGridScale,
			static_cast<float>(y) * gGridScale,
			0.0f
		};
	}

	/// <summary>
	/// グリッド座標をワールド座標に変換する（カスタムスケール版）。
	/// 特殊な用途でスケールを明示的に指定したい場合のみ使用。
	/// </summary>
	/// <param name="x">グリッドX座標</param>
	/// <param name="y">グリッドY座標</param>
	/// <param name="scale">グリッド1マス = 何ワールド単位か</param>
	inline No::Vector3 GridToWorld(int x, int y, float scale) {
		return No::Vector3{
			static_cast<float>(x) * scale,
			static_cast<float>(y) * scale,
			0.0f
		};
	}

	/// <summary>
	/// ワールド座標をグリッド座標に変換する
	/// </summary>
	/// <param name="worldPos">ワールド座標</param>
	/// <param name="outX">出力グリッドX座標</param>
	/// <param name="outY">出力グリッドY座標</param>
	inline void WorldToGrid(const No::Vector3& worldPos, int& outX, int& outY) {
		outX = static_cast<int>(std::round(worldPos.x / gGridScale));
		outY = static_cast<int>(std::round(worldPos.y / gGridScale));
	}

	/// <summary>
	/// ワールド座標をグリッド座標に変換する（カスタムスケール版）。
	/// </summary>
	/// <param name="worldPos">ワールド座標</param>
	/// <param name="scale">グリッド1マス = 何ワールド単位か</param>
	/// <param name="outX">出力グリッドX座標</param>
	/// <param name="outY">出力グリッドY座標</param>
	inline void WorldToGrid(const No::Vector3& worldPos, float scale, int& outX, int& outY) {
		outX = static_cast<int>(std::round(worldPos.x / scale));
		outY = static_cast<int>(std::round(worldPos.y / scale));
	}

	// ============================================================
	//  グリッドセル検索
	// ============================================================

	/// <summary>
	/// 指定座標のグリッドセルを取得する。
	/// 見つからない場合（マップ外）は nullptr を返す。
	/// 以前は PlayerMovementSystem / PlayerWeaponSystem / PlayerBulletSystem の
	/// 3か所に重複していたコードをここに集約。
	/// </summary>
	inline GridCellComponent* GetGridCell(No::Registry& registry, int x, int y) {
		auto view = registry.View<GridCellComponent>();
		for (auto entity : view) {
			auto* cell = registry.GetComponent<GridCellComponent>(entity);
			if (cell->gridX == x && cell->gridY == y) {
				return cell;
			}
		}
		return nullptr;
	}

	// ============================================================
	//  接続判定
	// ============================================================

	/// <summary>
	/// 指定方向への接続があるか返す。
	/// 以前は PlayerMovementSystem / PlayerWeaponSystem の2か所に重複していた。
	/// </summary>
	inline bool HasConnection(const GridCellComponent* cell, Direction dir) {
		switch (dir) {
		case Direction::Up:    return cell->hasConnectionUp;
		case Direction::Right: return cell->hasConnectionRight;
		case Direction::Down:  return cell->hasConnectionDown;
		case Direction::Left:  return cell->hasConnectionLeft;
		default:               return false;
		}
	}

	/// <summary>
	/// セルの接続数を数える。
	/// 行き止まり判定・交差点判定に使用する。
	/// </summary>
	inline int CountConnections(const GridCellComponent* cell) {
		int count = 0;
		if (cell->hasConnectionUp)    count++;
		if (cell->hasConnectionRight) count++;
		if (cell->hasConnectionDown)  count++;
		if (cell->hasConnectionLeft)  count++;
		return count;
	}

	// ============================================================
	//  方向ユーティリティ
	// ============================================================

	/// <summary>
	/// 指定方向の反対方向を返す。
	/// 後退禁止判定・BFS経路探索などで使用する。
	/// </summary>
	inline Direction GetOppositeDirection(Direction dir) {
		switch (dir) {
		case Direction::Up:    return Direction::Down;
		case Direction::Right: return Direction::Left;
		case Direction::Down:  return Direction::Up;
		case Direction::Left:  return Direction::Right;
		default:               return Direction::None;
		}
	}

	/// <summary>
	/// 指定座標と方向から、隣接ノード座標を計算する。
	/// </summary>
	inline void GetNextNodeCoords(int x, int y, Direction dir, int& outX, int& outY) {
		outX = x;
		outY = y;
		switch (dir) {
		case Direction::Up:    outY += 1; break;
		case Direction::Right: outX += 1; break;
		case Direction::Down:  outY -= 1; break;
		case Direction::Left:  outX -= 1; break;
		default:               break;
		}
	}


	// Player・Enemy 共通の移動可否判定（後退禁止 + 行き止まり例外 = A案）
	inline bool CanMoveInDirection(
		No::Registry& registry,
		int nodeX, int nodeY,
		Direction dir,
		Direction lastDir
	) {
		auto* cell = GetGridCell(registry, nodeX, nodeY);
		if (!cell) return false;
		if (!HasConnection(cell, dir)) return false;
		if (lastDir == Direction::None) return true;

		Direction opposite = GetOppositeDirection(lastDir);
		if (dir == opposite) {
			bool forwardBlocked = !HasConnection(cell, lastDir);
			bool onlyBackward = HasConnection(cell, opposite);
			int  connCount = CountConnections(cell);
			if (forwardBlocked && onlyBackward && connCount == 1) return true;
			return false;
		}
		return true;
	}

	/// <summary>
	/// プレイヤー専用の移動可否判定。
	/// CanMoveInDirection の判定に加え、移動先ノードが isEnemyOnly の場合は false を返す。
	/// PlayerMovementSystem の全ての移動判定で使用する。
	/// </summary>
	inline bool CanPlayerMoveInDirection(
		No::Registry& registry,
		int nodeX, int nodeY,
		Direction dir,
		Direction lastDir
	) {
		if (!CanMoveInDirection(registry, nodeX, nodeY, dir, lastDir)) return false;

		// 移動先ノードが敵専用道なら侵入不可
		int nx, ny;
		GetNextNodeCoords(nodeX, nodeY, dir, nx, ny);
		auto* targetCell = GetGridCell(registry, nx, ny);
		if (targetCell && targetCell->isEnemyOnly) return false;

		return true;
	}

} // namespace GridUtils