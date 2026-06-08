#include "CollisionFanc.h"
#include "CollisionHelper.h"
#include "../MathUtility.h"
#include "../Types/Calculations/Vector3Calculations.h"

namespace NoEngine {
namespace Math {
using namespace MathCalculations;
Math::Vector3 ClosestPointOnTriangle(const Math::Vector3& p, const Math::Vector3& a, const Math::Vector3& b, const Math::Vector3& c) {
	// 辺 AB
	Math::Vector3 ab = b - a;
	Math::Vector3 ac = c - a;
	Math::Vector3 ap = p - a;

	float d1 = Dot(ab, ap);
	float d2 = Dot(ac, ap);

	if (d1 <= 0.0f && d2 <= 0.0f) return a;

	// 辺 BC
	Math::Vector3 bp = p - b;
	float d3 = Dot(ab, bp);
	float d4 = Dot(ac, bp);

	if (d3 >= 0.0f && d4 <= d3) return b;

	// 辺 CA
	Math::Vector3 cp = p - c;
	float d5 = Dot(ab, cp);
	float d6 = Dot(ac, cp);

	if (d6 >= 0.0f && d5 <= d6) return c;

	// 領域 AB 内
	float vc = d1 * d4 - d3 * d2;
	if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f) {
		float v = d1 / (d1 - d3);
		return a + ab * v;
	}

	// 領域 AC 内
	float vb = d5 * d2 - d1 * d6;
	if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f) {
		float w = d2 / (d2 - d6);
		return a + ac * w;
	}

	// 領域 BC 内
	float va = d3 * d6 - d5 * d4;
	if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f) {
		float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
		return b + (c - b) * w;
	}

	// 内部領域（重心座標）
	float denom = 1.0f / (va + vb + vc);
	float v = vb * denom;
	float w = vc * denom;
	return a + ab * v + ac * w;
}
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

CapsuleTriangleCollision TestCapsuleTriangle(const Transform* capsuleTransform, const Math::CapsuleCollider* capsule, Math::TriangleCollider triangle) {
	CapsuleTriangleCollision result;

	// ワールド座標でのコライダーを取得
	CapsuleWorld cap = GetWorldCapsule(capsuleTransform, capsule);

	Vector3 segDir = cap.p1 - cap.p0;
	float segLenSq = segDir.LengthSquared();
	float t = 0.0f;

	if (segLenSq > 1e-8f) {
		Vector3 temp = triangle.Centroid() - cap.p0;
		t = temp.Dot(segDir) / segLenSq;
		t = std::clamp(t, 0.0f, 1.0f);
	}

	Math::Vector3 closestOnCapsule = cap.p0 + segDir * t;

	// 三角形上の最近接点
	Math::Vector3 closestOnTri = ClosestPointOnTriangle(
		closestOnCapsule,
		triangle.v[0], triangle.v[1], triangle.v[2]
	);

	// ---  距離を計算 ---
	Math::Vector3 diff = closestOnCapsule - closestOnTri;
	float dist = diff.Length();

	// ---  衝突判定 ---
	float r = capsule->radius;

	if (dist > r) {
		// 衝突していない
		return result;
	}

	// ---  衝突している場合 ---
	result.hit = true;
	result.closestOnCapsule = closestOnCapsule;
	result.closestOnTriangle = closestOnTri;

	// 法線（triangle → capsule）
	if (dist > 1e-8f) {
		result.normal = diff / dist;
	} else {
		// diff がゼロに近い場合は三角形法線を使う
		result.normal = triangle.normal;
	}

	result.penetration = r - dist;
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

bool IsCollision(const Vector2& positon, const AABBCollider2D* aabb, const Transform2D* transform) {
	AABBWorld2D box = GetWorldAABB2D(transform, aabb);

	if (box.min.x < positon.x && positon.x < box.max.x) {
		if (box.min.y < positon.y && positon.y < box.max.y) {
			return true;
		}
	}
	return false;
}

}
}