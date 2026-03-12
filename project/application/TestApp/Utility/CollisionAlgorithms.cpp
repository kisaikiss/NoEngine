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

}