#pragma once
#include "engine/Math/MathInclude.h"
#include <unordered_set>

/// <summary>
/// プレイヤーの弾丸コンポーネント
/// 空間上を直進し、グリッド上で進行できない地点に到達すると消滅する
/// </summary>
struct PlayerBulletComponent {
	NoEngine::Math::Vector3 direction;	// 進行方向（正規化済み）
	int startNodeX;						// 発射元のグリッドX座標（このノードのみグリッド判定をスキップする）
	int startNodeY;						// 発射元のグリッドY座標
	float speed;						// 移動速度
	float travelDistance;				// 移動した距離（発射元ノード判定用）
	
	std::unordered_set<int> visitedIntersections;	// 訪問済み交差点（重複発生防止用）

	// ---- 弾の挙動制御フラグ ----
	bool penetrateWalls;				// 壁を貫通するか
	bool penetrateEnemies;				// 敵を貫通するか
	bool enableLooping;					// 画面外ループを有効にするか
	bool disableLoopOnHit;				// 敵に当たった時にループを無効化するか

	// ---- ループ弾識別 ----
	bool isLoopedBullet;				// ループ弾かどうか（通常弾=false、ループ弾=true）
	
	float screenBoundsOffset;			// 画面外判定のオフセット（モデルサイズ分の余裕）

	PlayerBulletComponent()
		: direction(NoEngine::Math::Vector3::ZERO),
		startNodeX(0),
		startNodeY(0),
		speed(5.0f),
		travelDistance(0.0f),
		penetrateWalls(false),
		penetrateEnemies(false),
		enableLooping(false),
		disableLoopOnHit(false),
		isLoopedBullet(false),
		screenBoundsOffset(0.5f) {
	}
};