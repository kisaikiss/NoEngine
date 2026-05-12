#include "CollisionFanc.h"
#include "CollisionHelper.h"
#include "../MathUtility.h"

namespace NoEngine {
namespace Math {

CapsuleAABBCollision TestCapsuleAABB(const Transform* capsuleTransform, const Math::CapsuleCollider* capsule,
	const Transform* aabbTransform, const Math::AABBCollider* aabb) {
	CapsuleAABBCollision result;

	// ワールド座標でのそれぞれのコライダーを取得
	CapsuleWorld cap = GetWorldCapsule(capsuleTransform, capsule);
	AABBWorld box = GetWorldAABB(aabbTransform, aabb);

	// カプセルの中心軸を表すベクトル
	Vector3 segment = cap.p1 - cap.p0;
	// AABBの中心
	Vector3 boxCenter = 0.5f * (box.max + box.min);

	// AABBの中心からカプセル下端へのベクトル
	Vector3 p0ToCenter = boxCenter - cap.p0;

	float segmentLengthSquared = segment.LengthSquared();

	// 線分への射影でtを求める
	float t = 0.0f;
	if (segmentLengthSquared > 0.0f) {
		t = p0ToCenter.Dot(segment) / segmentLengthSquared;
	}
	// 線分範囲へclamp
	t = std::clamp(t, 0.0f, 1.0f);

	// 線分上の最近接点を求める
	Vector3 pointOnSegment = cap.p0 + segment * t;
	// AABBにclampして最近接点を求める
	Vector3 closestOnBox = Clamp(pointOnSegment, box.min, box.max);

	// 差分ベクトル最近接点間の距離。
	Vector3 diff = pointOnSegment - closestOnBox;

	// 0除算を避けるために1e-8fでclamp
	float dist = std::max(diff.Length(), 1e-8f);
	float r = cap.radius;

	// 衝突判定(最近接点間の距離が半径より小さいか)
	if (dist <= r) {
		// 衝突している
		result.hit = true;
		// 貫通量
		result.penetration = r - dist;
		// 法線ベクトル
		result.normal = diff / dist;

		result.closestOnBox = closestOnBox;
		result.closestOnCapsule = pointOnSegment;
	}

	return result;
}

Collision2D TestAABB2D(const Transform2D* transformA, const Math::AABBCollider2D* aabbA, const Transform2D* transformB, const Math::AABBCollider2D* aabbB) {
	AABBWorld2D boxA = GetWorldAABB2D(transformA, aabbA);
	AABBWorld2D boxB = GetWorldAABB2D(transformB, aabbB);

	Collision2D result;

	// 軸ごとの重なり量
	float overlapX = std::min(boxA.max.x, boxB.max.x) - std::max(boxA.min.x, boxB.min.x);
	float overlapY = std::min(boxA.max.y, boxB.max.y) - std::max(boxA.min.y, boxB.min.y);

	// どちらかの軸で重なっていなければ衝突なし
	if (overlapX <= 0.0f || overlapY <= 0.0f) {
		return result;
	}

	result.hit = true;

	// AABBの中心
	Vector2 centerA = 0.5f * (boxA.max + boxA.min);
	Vector2 centerB = 0.5f * (boxB.max + boxB.min);

	// 最小の重なり軸で押し戻す（MTV）
	if (overlapX <= overlapY) {
		// X方向
		float dir = (centerA.x < centerB.x) ? -1.0f : 1.0f;
		result.penetration = overlapX;
		result.normal = { dir, 0 };

		// 最近接点
		result.closestOnA = { (dir < 0 ? boxA.max.x : boxA.min.x), centerA.y };
		result.closestOnB = { (dir < 0 ? boxB.min.x : boxB.max.x), centerB.y };

	} else {
		// Y方向
		float dir = (centerA.y < centerB.y) ? -1.0f : 1.0f;
		result.penetration = overlapY;
		result.normal = { 0, dir };

		result.closestOnA = { centerA.x, (dir < 0 ? boxA.max.y : boxA.min.y) };
		result.closestOnB = { centerB.x, (dir < 0 ? boxB.min.y : boxB.max.y) };
	}

	return result;
}

Collision2D TestAABB2DHorizontal(const Transform2D* transformA, const Math::AABBCollider2D* aabbA, const Transform2D* transformB, const Math::AABBCollider2D* aabbB) {
	AABBWorld2D boxA = GetWorldAABB2D(transformA, aabbA);
	AABBWorld2D boxB = GetWorldAABB2D(transformB, aabbB);

	Collision2D result;

	// 軸ごとの重なり量
	float overlapX = std::min(boxA.max.x, boxB.max.x) - std::max(boxA.min.x, boxB.min.x);
	float overlapY = std::min(boxA.max.y, boxB.max.y) - std::max(boxA.min.y, boxB.min.y);

	// どちらかの軸で重なっていなければ衝突なし
	if (overlapX <= 0.0f || overlapY <= 0.0f) {
		return result;
	}

	result.hit = true;

	// AABBの中心
	Vector2 centerA = 0.5f * (boxA.max + boxA.min);
	Vector2 centerB = 0.5f * (boxB.max + boxB.min);

	// 最小の重なり軸で押し戻す（MTV）

	// X方向
	float dir = (centerA.x < centerB.x) ? -1.0f : 1.0f;
	result.penetration = overlapX;
	result.normal = { dir, 0 };

	// 最近接点
	result.closestOnA = { (dir < 0 ? boxA.max.x : boxA.min.x), centerA.y };
	result.closestOnB = { (dir < 0 ? boxB.min.x : boxB.max.x), centerB.y };


	return result;
}

Collision2D TestAABB2DVertical(const Transform2D* transformA, const Math::AABBCollider2D* aabbA, const Transform2D* transformB, const Math::AABBCollider2D* aabbB) {
	AABBWorld2D boxA = GetWorldAABB2D(transformA, aabbA);
	AABBWorld2D boxB = GetWorldAABB2D(transformB, aabbB);

	Collision2D result;

	// 軸ごとの重なり量
	float overlapX = std::min(boxA.max.x, boxB.max.x) - std::max(boxA.min.x, boxB.min.x);
	float overlapY = std::min(boxA.max.y, boxB.max.y) - std::max(boxA.min.y, boxB.min.y);

	// どちらかの軸で重なっていなければ衝突なし
	if (overlapX <= 0.0f || overlapY <= 0.0f) {
		return result;
	}

	result.hit = true;

	// AABBの中心
	Vector2 centerA = 0.5f * (boxA.max + boxA.min);
	Vector2 centerB = 0.5f * (boxB.max + boxB.min);

	// 最小の重なり軸で押し戻す（MTV）

	// Y方向
	float dir = (centerA.y < centerB.y) ? -1.0f : 1.0f;
	result.penetration = overlapY;
	result.normal = { 0, dir };

	result.closestOnA = { centerA.x, (dir < 0 ? boxA.max.y : boxA.min.y) };
	result.closestOnB = { centerB.x, (dir < 0 ? boxB.min.y : boxB.max.y) };


	return result;
}

}
}