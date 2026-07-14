#include "CollisionFanc.h"
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
	const Transform* aabbTransform, const Math::AABBCollider* aabb, ECS::Registry& registry) {
	CapsuleAABBCollision result;

	// ワールド座標でのそれぞれのコライダーを取得
	CapsuleWorld cap = GetWorldCapsule(capsuleTransform, capsule, registry);
	AABBWorld box = GetWorldAABB(aabbTransform, aabb, registry);

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
CapsuleTriangleCollision TestCapsuleTriangle(
	const Transform* capsuleTransform,
	const Math::CapsuleCollider* capsule,
	Math::TriangleCollider triangle, ECS::Registry& registry) {
	CapsuleTriangleCollision result;

	CapsuleWorld cap = GetWorldCapsule(capsuleTransform, capsule, registry);

	Vector3 segA = cap.p0;
	Vector3 segB = cap.p1;
	Vector3 A = triangle.v[0];
	Vector3 B = triangle.v[1];
	Vector3 C = triangle.v[2];
	float   r = capsule->radius;

	// ----------------------------------------------------------------
	//  Step 1. カプセル線分の各端点 → 三角形 の最近接点を調べる
	//          （線分が面に突き刺さるケースをここで捕捉）
	// ----------------------------------------------------------------
	auto testPoint = [&](const Vector3& pt) -> std::pair<Vector3, float> {
		Vector3 onTri = ClosestPointOnTriangle(pt, A, B, C);
		float   dist = (pt - onTri).Length();
		return { onTri, dist };
		};

	float   bestDist = 1e30f;
	Vector3 bestOnCap = segA;
	Vector3 bestOnTri = A;

	auto tryPair = [&](const Vector3& onCap, const Vector3& onTri) {
		float d = (onCap - onTri).Length();
		if (d < bestDist) {
			bestDist = d;
			bestOnCap = onCap;
			bestOnTri = onTri;
		}
		};

	// 端点 p0, p1 から三角形への最近接
	{
		auto [onTri0, d0] = testPoint(segA);
		tryPair(segA, onTri0);
		auto [onTri1, d1] = testPoint(segB);
		tryPair(segB, onTri1);
		// 中点も追加（急斜面で線分が三角形を跨ぐケース）
		Vector3 mid = (segA + segB) * 0.5f;
		auto [onTriM, dM] = testPoint(mid);
		tryPair(mid, onTriM);
	}

	// ----------------------------------------------------------------
	//  Step 2. カプセル線分 × 三角形の3辺 の最近接点ペアを調べる
	//          （辺方向の接触はここで捕捉）
	// ----------------------------------------------------------------
	auto closestSegSeg = [&](
		const Vector3& p0, const Vector3& p1,   // カプセル線分
		const Vector3& q0, const Vector3& q1,   // 三角形の辺
		Vector3& outOnCap, Vector3& outOnEdge) {
			Vector3 d1 = p1 - p0;
			Vector3 d2 = q1 - q0;
			Vector3 r_ = p0 - q0;

			float a = d1.Dot(d1);
			float e = d2.Dot(d2);
			float f = d2.Dot(r_);

			float s, t;

			if (a < 1e-8f && e < 1e-8f) {
				// 両方縮退
				s = t = 0.0f;
			} else if (a < 1e-8f) {
				// p が縮退
				s = 0.0f;
				t = std::clamp(f / e, 0.0f, 1.0f);
			} else {
				float c = d1.Dot(r_);
				if (e < 1e-8f) {
					// q が縮退
					t = 0.0f;
					s = std::clamp(-c / a, 0.0f, 1.0f);
				} else {
					float b_ = d1.Dot(d2);
					float denom = a * e - b_ * b_;

					if (std::abs(denom) > 1e-8f) {
						s = std::clamp((b_ * f - c * e) / denom, 0.0f, 1.0f);
					} else {
						s = 0.0f;
					}

					t = (b_ * s + f) / e;

					if (t < 0.0f) {
						t = 0.0f;
						s = std::clamp(-c / a, 0.0f, 1.0f);
					} else if (t > 1.0f) {
						t = 1.0f;
						s = std::clamp((b_ - c) / a, 0.0f, 1.0f);
					}
				}
			}

			outOnCap = p0 + d1 * s;
			outOnEdge = q0 + d2 * t;
		};

	// 3辺それぞれと比較
	auto tryEdge = [&](const Vector3& q0, const Vector3& q1) {
		Vector3 onCap, onEdge;
		closestSegSeg(segA, segB, q0, q1, onCap, onEdge);
		tryPair(onCap, onEdge);
		};

	tryEdge(A, B);
	tryEdge(B, C);
	tryEdge(C, A);

	// ----------------------------------------------------------------
	//  Step 3. 衝突判定
	// ----------------------------------------------------------------
	if (bestDist > r) return result;   // 衝突なし

	// ----------------------------------------------------------------
	//  Step 4. 法線と貫通量
	// ----------------------------------------------------------------
	result.hit = true;
	result.closestOnCapsule = bestOnCap;
	result.closestOnTriangle = bestOnTri;
	result.penetration = r - bestDist;

	if (bestDist > 1e-6f) {
		result.normal = (bestOnCap - bestOnTri) / bestDist;
	} else {
		// 完全貫通 → 面法線で押し出す（裏面貫通対策で符号を確認）
		Vector3 n = triangle.normal;
		// カプセル重心が裏側にあれば反転
		Vector3 capsuleCenter = (segA + segB) * 0.5f;
		if ((capsuleCenter - A).Dot(n) < 0.0f) n = n * -1.0f;
		result.normal = n;
	}

	return result;
}

CapsuleSphereCollision TestCapsuleSphere(const Transform* capsuleTransform, const Math::CapsuleCollider* capsule,
	const Transform* sphereTransform, const Math::SphereCollider* sphere, ECS::Registry& registry) {
	CapsuleSphereCollision result{};
	CapsuleWorld cap = GetWorldCapsule(capsuleTransform, capsule, registry);
	SphereWorld sph = GetWorldSphere(sphereTransform, sphere, registry);


	Vector3 v = cap.p1 - cap.p0;    // 線分の方向ベクトル
	Vector3 w = sph.center - cap.p0;     // 点からp0への方向ベクトル

	// 線分の長さの2乗
	float l = v.LengthSquared();

	// 投影比率 t を内積から計算
	float t = w.Dot(v) / l;

	// 線分の範囲内にクランプ
	t = std::clamp(t, 0.0f, 1.0f);

	//  線分上の最近接点を求める
	Vector3 nearPoint = cap.p0 + v * t;

	// 距離を算出
	float length = (sph.center - nearPoint).LengthSquared();

	// 半径の合計以下かどうかを返す
	result.hit = length <= (cap.radius + sph.radius) * (cap.radius + sph.radius);
	if (!result.hit) return result;

	// ToDo: 押し戻し用の貫通量や法線の計算を入れる
	

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