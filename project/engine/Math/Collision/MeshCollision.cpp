#include "CollisionFanc.h"
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
CapsuleTriangleCollision TestSegmentTriangle(const Vector3& segA, const Vector3& segB, float radius, const TriangleCollider& triangle) {
	CapsuleTriangleCollision result;

	Vector3 A = triangle.v[0];
	Vector3 B = triangle.v[1];
	Vector3 C = triangle.v[2];
	float   r = radius;
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

	// 線分が三角形面を横切る場合、両端点や中点の最近接点だけでは
	// 面との交差を取りこぼすことがあるため、線分と面の交点も調べる。
	Vector3 planeNormal = triangle.normal;
	float planeNormalLength = planeNormal.Length();
	if (planeNormalLength > 1e-6f) {
		planeNormal = planeNormal / planeNormalLength;
		float distanceA = (segA - A).Dot(planeNormal);
		float distanceB = (segB - A).Dot(planeNormal);
		float denominator = distanceA - distanceB;
		if (distanceA * distanceB <= 0.0f && std::abs(denominator) > 1e-8f) {
			float t = std::clamp(distanceA / denominator, 0.0f, 1.0f);
			Vector3 intersection = segA + (segB - segA) * t;
			Vector3 onTriangle = ClosestPointOnTriangle(intersection, A, B, C);
			if ((intersection - onTriangle).LengthSquared() <= 1e-8f) {
				tryPair(intersection, onTriangle);
			}
		}
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
	result.distance = bestDist;
	result.closestOnCapsule = bestOnCap;
	result.closestOnTriangle = bestOnTri;
	if (bestDist > 1e-6f) result.normal = (bestOnCap - bestOnTri) / bestDist;
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
		// 線分が面を横切る場合は、カプセル中心がある側へ向けて押し出す。
		Vector3 n = triangle.normal;
		float normalLength = n.Length();
		if (normalLength <= 1e-6f) return CapsuleTriangleCollision{};
		n = n / normalLength;
		float signedA = (segA - A).Dot(n);
		float signedB = (segB - A).Dot(n);
		if (signedA + signedB < 0.0f) n = n * -1.0f;
		result.normal = n;
		// 線分の反対側の端まで面の外に出す深さを含め、地面を通り抜けないようにする。
		float minSignedDistance = std::min((segA - A).Dot(n), (segB - A).Dot(n));
		result.penetration = r - minSignedDistance;
	}

	return result;
}

namespace {
constexpr float kTerrainSkin = 0.0001f;

AABBCollider SweptCapsuleBounds(const CapsuleWorld& capsule, const Vector3& displacement) {
	AABBCollider bounds;
	bounds.Expand(capsule.p0);
	bounds.Expand(capsule.p1);
	bounds.Expand(capsule.p0 + displacement);
	bounds.Expand(capsule.p1 + displacement);
	const float r = capsule.radius + kTerrainSkin;
	bounds.min -= Vector3{ r, r, r };
	bounds.max += Vector3{ r, r, r };
	return bounds;
}

bool IsSolidTerrain(ECS::Registry& registry, ECS::Entity entity) {
	auto* body = registry.GetComponent<Component::CollisionBody>(entity);
	return !body || body->type != Component::BodyType::Through;
}
}

CapsuleSweepHit SweepCapsuleTriangle(const CapsuleWorld& capsule, const Vector3& displacement,
	const TriangleCollider& triangle) {
	CapsuleSweepHit result;
	float fraction = 0.0f;
	Vector3 normal = Vector3::ZERO;
	// 保守的な前進（conservative advancement）では、最も近い点における分離平面を用いる。
	// 端点同士の重なりを調べる手法とは異なり、この手法では薄い三角形を飛び越えてしまうことはない。
	for (int iteration = 0; iteration < 64; ++iteration) {
		const Vector3 offset = displacement * fraction;
		const auto contact = TestSegmentTriangle(capsule.p0 + offset, capsule.p1 + offset,
			capsule.radius, triangle);
		normal = contact.normal;
		const float closing = -displacement.Dot(normal);
		// A touching capsule must still be able to move along or away from a wall.
		if (closing <= 1e-8f) return result;
		const float gap = contact.distance - capsule.radius;
		if (gap <= kTerrainSkin) return { true, fraction, normal };
		const float step = (gap - kTerrainSkin * 0.5f) / closing;
		if (fraction + step > 1.0f) return result;
		if (step <= 1e-7f) return { true, fraction, normal };
		fraction += step;
	}
	// 接触（かすめるような接触）が収束しない場合は、最後に確認された安全な位置を保持する。
	return { true, fraction, normal };
}

CapsuleSweepHit SweepCapsuleTerrain(ECS::Registry& registry, const CapsuleWorld& capsule,
	const Vector3& displacement) {
	CapsuleSweepHit nearest;
	const auto bounds = SweptCapsuleBounds(capsule, displacement);
	std::vector<int> candidates;
	for (auto entity : registry.View<TerrainMesh>()) {
		if (!IsSolidTerrain(registry, entity)) continue;
		auto* terrain = registry.GetComponent<TerrainMesh>(entity);
		candidates.clear();
		QueryBVH(terrain->bvhRoot.get(), bounds, terrain->triangles, candidates);
		for (int index : candidates) {
			const auto hit = SweepCapsuleTriangle(capsule, displacement, terrain->triangles[index]);
			if (hit.hit && (!nearest.hit || hit.fraction < nearest.fraction)) nearest = hit;
		}
	}
	return nearest;
}

Vector3 MoveCapsuleTerrain(ECS::Registry& registry, CapsuleWorld capsule,
	const Vector3& displacement, Vector3* velocity, float groundSnapDistance, float minGroundNormalY) {
	Vector3 moved = Vector3::ZERO;
	std::vector<int> candidates;
	// 修正のたびに再クエリを行うこと。ある面を押し出すと、別の面に影響が及ぶ可能性があるためである。
	for (int iteration = 0; iteration < 8; ++iteration) {
		CapsuleTriangleCollision deepest;
		const auto bounds = SweptCapsuleBounds(capsule, Vector3::ZERO);
		for (auto entity : registry.View<TerrainMesh>()) {
			if (!IsSolidTerrain(registry, entity)) continue;
			auto* terrain = registry.GetComponent<TerrainMesh>(entity);
			candidates.clear();
			QueryBVH(terrain->bvhRoot.get(), bounds, terrain->triangles, candidates);
			for (int index : candidates) {
				auto contact = TestSegmentTriangle(capsule.p0, capsule.p1, capsule.radius,
					terrain->triangles[index]);
				if (contact.hit && contact.penetration > deepest.penetration) deepest = contact;
			}
		}
		if (deepest.penetration <= 0.0f) break;
		const Vector3 correction = deepest.normal * (deepest.penetration + kTerrainSkin);
		capsule.p0 += correction;
		capsule.p1 += correction;
		moved += correction;
	}

	Vector3 remaining = displacement;
	std::vector<Vector3> normals;
	for (int iteration = 0; iteration < 8 && remaining.LengthSquared() > 1e-12f; ++iteration) {
		const auto hit = SweepCapsuleTerrain(registry, capsule, remaining);
		const Vector3 step = remaining * hit.fraction;
		capsule.p0 += step;
		capsule.p1 += step;
		moved += step;
		if (!hit.hit) break;
		remaining *= 1.0f - hit.fraction;
		normals.push_back(hit.normal);
		// 以前の面を再適用し、後から現れるスライドが角に戻り込まないようにする。
		for (int pass = 0; pass < 4; ++pass) {
			for (const auto& normal : normals) {
				const float inward = remaining.Dot(normal);
				if (inward < 0.0f) remaining -= normal * inward;
				if (velocity) {
					const float speed = velocity->Dot(normal);
					if (speed < 0.0f) *velocity -= normal * speed;
				}
			}
		}
	}
	// 移動先の足元をカプセル全体で調べ、曲面で接平面から離れた分だけ下へ追従する。
	// 最初の障害物が壁や急斜面なら、その先にある地面には吸着しない。
	if (groundSnapDistance > 0.f) {
		const Vector3 down{ 0.f, -groundSnapDistance, 0.f };
		const auto ground = SweepCapsuleTerrain(registry, capsule, down);
		if (ground.hit && ground.normal.y >= minGroundNormalY) {
			moved += down * ground.fraction;
		}
	}
	return moved;
}

}
}
