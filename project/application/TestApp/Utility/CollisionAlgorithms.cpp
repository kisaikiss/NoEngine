#include "CollisionAlgorithms.h"
#include <algorithm>
#include <cmath>

namespace TestApp {

	bool CollisionAlgorithms::CheckCircleCircle(
		const No::Vector2& center1, float radius1,
		const No::Vector2& center2, float radius2
	) {

		// 中心間の距離の二乗を計算
		// 平方根を避けて高速化（距離の二乗で比較）
		float dx = center1.x - center2.x;
		float dy = center1.y - center2.y;
		float distanceSquared = dx * dx + dy * dy;


		// 半径の和の二乗を計算
		float radiusSum = radius1 + radius2;
		float radiusSumSquared = radiusSum * radiusSum;


		// 距離が半径の和以下なら衝突
		return distanceSquared <= radiusSumSquared;
	}

	bool CollisionAlgorithms::CheckCircleAABB(
		const No::Vector2& circleCenter, float radius,
		const No::Vector2& rectCenter, const No::Vector2& rectSize
	) {

		// AABBのハーフサイズを計算
		// rectSizeはフルサイズなので、中心からの距離に変換
		float halfWidth = rectSize.x * 0.5f;
		float halfHeight = rectSize.y * 0.5f;


		// AABBの範囲を計算
		float left = rectCenter.x - halfWidth;
		float right = rectCenter.x + halfWidth;
		float top = rectCenter.y - halfHeight;
		float bottom = rectCenter.y + halfHeight;


		// 円の中心からAABBの最近接点を求める
		// clamp関数で各軸の最も近い点を取得
		float closestX = std::clamp(circleCenter.x, left, right);
		float closestY = std::clamp(circleCenter.y, top, bottom);


		// 円の中心と最近接点の距離を計算
		float dx = circleCenter.x - closestX;
		float dy = circleCenter.y - closestY;
		float distanceSquared = dx * dx + dy * dy;


		// 距離が半径以下なら衝突
		return distanceSquared <= (radius * radius);
	}

	bool CollisionAlgorithms::CheckAABBAABB(
		const No::Vector2& center1, const No::Vector2& size1,
		const No::Vector2& center2, const No::Vector2& size2
	) {

		// 各AABBのハーフサイズを計算
		float halfWidth1 = size1.x * 0.5f;
		float halfHeight1 = size1.y * 0.5f;
		float halfWidth2 = size2.x * 0.5f;
		float halfHeight2 = size2.y * 0.5f;


		// AABB1の範囲を計算
		float left1 = center1.x - halfWidth1;
		float right1 = center1.x + halfWidth1;
		float top1 = center1.y - halfHeight1;
		float bottom1 = center1.y + halfHeight1;


		// AABB2の範囲を計算
		float left2 = center2.x - halfWidth2;
		float right2 = center2.x + halfWidth2;
		float top2 = center2.y - halfHeight2;
		float bottom2 = center2.y + halfHeight2;


		// 各軸で範囲が重なっているかチェック
		// X軸とY軸の両方で重なっていれば衝突
		bool overlapX = left1 <= right2 && right1 >= left2;
		bool overlapY = top1 <= bottom2 && bottom1 >= top2;

		return overlapX && overlapY;
	}

	bool CollisionAlgorithms::CheckSphereSphere(
		const No::Vector3& center1, float radius1,
		const No::Vector3& center2, float radius2
	) {

		// 中心間の距離の二乗を計算
		float dx = center1.x - center2.x;
		float dy = center1.y - center2.y;
		float dz = center1.z - center2.z;
		float distanceSquared = dx * dx + dy * dy + dz * dz;


		// 半径の和の二乗を計算
		float radiusSum = radius1 + radius2;
		float radiusSumSquared = radiusSum * radiusSum;


		// 距離が半径の和以下なら衝突
		return distanceSquared <= radiusSumSquared;
	}

	bool CollisionAlgorithms::CheckSphereAABB3D(
		const No::Vector3& sphereCenter, float radius,
		const No::Vector3& boxCenter, const No::Vector3& boxSize
	) {
		// AABBのハーフサイズを計算
		float halfX = boxSize.x * 0.5f;
		float halfY = boxSize.y * 0.5f;
		float halfZ = boxSize.z * 0.5f;

		// AABBの各軸の範囲
		float minX = boxCenter.x - halfX;
		float maxX = boxCenter.x + halfX;
		float minY = boxCenter.y - halfY;
		float maxY = boxCenter.y + halfY;
		float minZ = boxCenter.z - halfZ;
		float maxZ = boxCenter.z + halfZ;

		// 球体の中心からAABBの最近接点を求める
		float closestX = std::clamp(sphereCenter.x, minX, maxX);
		float closestY = std::clamp(sphereCenter.y, minY, maxY);
		float closestZ = std::clamp(sphereCenter.z, minZ, maxZ);

		// 球体の中心と最近接点の距離の二乗を計算
		float dx = sphereCenter.x - closestX;
		float dy = sphereCenter.y - closestY;
		float dz = sphereCenter.z - closestZ;
		float distanceSquared = dx * dx + dy * dy + dz * dz;

		// 距離が半径以下なら衝突
		return distanceSquared <= (radius * radius);
	}

	bool CollisionAlgorithms::CheckAABB3DAABB3D(
		const No::Vector3& center1, const No::Vector3& size1,
		const No::Vector3& center2, const No::Vector3& size2
	) {
		// 各AABBのハーフサイズを計算
		float halfX1 = size1.x * 0.5f;
		float halfY1 = size1.y * 0.5f;
		float halfZ1 = size1.z * 0.5f;
		float halfX2 = size2.x * 0.5f;
		float halfY2 = size2.y * 0.5f;
		float halfZ2 = size2.z * 0.5f;

		// 各軸で範囲が重なっているかチェック
		// X / Y / Z の全軸で重なっていれば衝突
		bool overlapX = (center1.x - halfX1) <= (center2.x + halfX2) &&
			(center1.x + halfX1) >= (center2.x - halfX2);
		bool overlapY = (center1.y - halfY1) <= (center2.y + halfY2) &&
			(center1.y + halfY1) >= (center2.y - halfY2);
		bool overlapZ = (center1.z - halfZ1) <= (center2.z + halfZ2) &&
			(center1.z + halfZ1) >= (center2.z - halfZ2);

		return overlapX && overlapY && overlapZ;
	}


	// ============================================================
	// スクリーン投影衝突判定
	// ============================================================

	std::vector<No::Vector2> CollisionAlgorithms::ComputeConvexHull(
		std::vector<No::Vector2> pts
	) {
		int n = static_cast<int>(pts.size());

		// 3点未満は凸包を作れないのでそのまま返す
		if (n <= 2) return pts;

		// ========================================
		// 基準点の選定
		// スクリーン座標はY軸が下向き
		// → Y が最小（画面の最上）の点を基準とし、同率なら X が最小の点
		// ========================================
		int pivotIdx = 0;
		for (int i = 1; i < n; ++i) {
			if (pts[i].y < pts[pivotIdx].y ||
				(pts[i].y == pts[pivotIdx].y && pts[i].x < pts[pivotIdx].x)) {
				pivotIdx = i;
			}
		}
		std::swap(pts[0], pts[pivotIdx]);
		const No::Vector2 p0 = pts[0];

		// ========================================
		// 極角でソート
		// 外積 ax*by - ay*bx > 0 → a が b より「左」（反時計回り方向に先）
		// スクリーン座標（Y下向き）では見た目は時計回りになるが
		// SAT 判定には向きは影響しないため問題なし
		// ========================================
		std::sort(pts.begin() + 1, pts.end(), [&](const No::Vector2& a, const No::Vector2& b) {
			const float ax = a.x - p0.x, ay = a.y - p0.y;
			const float bx = b.x - p0.x, by = b.y - p0.y;
			const float cross = ax * by - ay * bx;
			if (std::abs(cross) > 1e-6f) return cross > 0.0f;
			// 同角度の場合: 基準点に近い点を先にする（遠い点は後で除去）
			return (ax * ax + ay * ay) < (bx * bx + by * by);
			});

		// ========================================
		// Graham Scan
		// 外積 <= 0（右折または直線）の点はスタックから除去
		// ========================================
		std::vector<No::Vector2> hull;
		hull.reserve(n);
		for (const auto& p : pts) {
			while (hull.size() >= 2) {
				const No::Vector2& a = hull[hull.size() - 2];
				const No::Vector2& b = hull[hull.size() - 1];
				// (b-a) × (p-a)
				const float cross =
					(b.x - a.x) * (p.y - a.y) -
					(b.y - a.y) * (p.x - a.x);
				if (cross <= 0.0f) {
					hull.pop_back(); // 右折または直線 → 除去
				} else {
					break;
				}
			}
			hull.push_back(p);
		}
		return hull;
	}


	bool CollisionAlgorithms::CheckConvexHullAABB(
		const std::vector<No::Vector2>& hull,
		const No::Vector2& rectCenter,
		const No::Vector2& rectSize
	) {
		// 凸包が空なら衝突なし
		if (hull.empty()) return false;

		const float halfW = rectSize.x * 0.5f;
		const float halfH = rectSize.y * 0.5f;

		// AABB の4頂点（SAT の投影に使用）
		const No::Vector2 aabbVerts[4] = {
			{ rectCenter.x - halfW, rectCenter.y - halfH },
			{ rectCenter.x + halfW, rectCenter.y - halfH },
			{ rectCenter.x + halfW, rectCenter.y + halfH },
			{ rectCenter.x - halfW, rectCenter.y + halfH },
		};

		// ========================================
		// AABB の軸（X軸 / Y軸）で分離判定
		// 凸包の外接 AABB vs スプライト AABB の判定と同等
		// ========================================

		// X 軸
		{
			float hMin = hull[0].x, hMax = hull[0].x;
			for (const auto& p : hull) {
				hMin = std::min(hMin, p.x);
				hMax = std::max(hMax, p.x);
			}
			const float aMin = rectCenter.x - halfW;
			const float aMax = rectCenter.x + halfW;
			if (hMax < aMin || aMax < hMin) return false; // 分離軸発見 → 衝突なし
		}

		// Y 軸
		{
			float hMin = hull[0].y, hMax = hull[0].y;
			for (const auto& p : hull) {
				hMin = std::min(hMin, p.y);
				hMax = std::max(hMax, p.y);
			}
			const float aMin = rectCenter.y - halfH;
			const float aMax = rectCenter.y + halfH;
			if (hMax < aMin || aMax < hMin) return false; // 分離軸発見 → 衝突なし
		}

		// ========================================
		// 凸包の各辺の法線軸で分離判定
		// これが AABB 判定との違い:
		//   斜め辺の法線方向でも分離できるか検査することで
		//   外接 AABB の「四隅の余白」による誤判定を排除する
		// ========================================
		const int hullSize = static_cast<int>(hull.size());
		for (int i = 0; i < hullSize; ++i) {
			const No::Vector2& cur = hull[i];
			const No::Vector2& nxt = hull[(i + 1) % hullSize];

			// 辺ベクトルの法線（正規化不要: SAT は比率で比較するため）
			const float nx = -(nxt.y - cur.y);
			const float ny = (nxt.x - cur.x);

			// 凸包を法線軸に投影
			float hMin = nx * hull[0].x + ny * hull[0].y;
			float hMax = hMin;
			for (const auto& p : hull) {
				const float proj = nx * p.x + ny * p.y;
				hMin = std::min(hMin, proj);
				hMax = std::max(hMax, proj);
			}

			// AABB を法線軸に投影
			float aMin = nx * aabbVerts[0].x + ny * aabbVerts[0].y;
			float aMax = aMin;
			for (const auto& v : aabbVerts) {
				const float proj = nx * v.x + ny * v.y;
				aMin = std::min(aMin, proj);
				aMax = std::max(aMax, proj);
			}

			// 分離軸発見 → 衝突なし
			if (hMax < aMin || aMax < hMin) return false;
		}

		// 全ての分離軸で重なりが確認された → 衝突
		return true;
	}

}