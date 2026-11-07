#include "CollisionFanc.h"
#include "../MathUtility.h"
#include "../Types/Calculations/Vector3Calculations.h"

namespace NoEngine {
namespace Math {
using namespace MathCalculations;

CapsuleCapsuleCollision TestCapsuleCapsule(const CapsuleWorld& capsuleA, const CapsuleWorld& capsuleB) {
	CapsuleCapsuleCollision result;
	if (!std::isfinite(capsuleA.radius) || !std::isfinite(capsuleB.radius)
		|| capsuleA.radius < 0.0f || capsuleB.radius < 0.0f) return result;
	const Vector3 axisA = capsuleA.p1 - capsuleA.p0;
	const Vector3 axisB = capsuleB.p1 - capsuleB.p0;
	const Vector3 offset = capsuleA.p0 - capsuleB.p0;
	const float lengthA = axisA.LengthSquared();
	const float lengthB = axisB.LengthSquared();
	const float projectionA = axisA.Dot(offset);
	const float projectionB = axisB.Dot(offset);
	constexpr float epsilon = 1e-8f;
	float parameterA = 0.0f;
	float parameterB = 0.0f;

	if (lengthA <= epsilon && lengthB <= epsilon) {
		// 両方の中心軸が点に縮退した場合は、球同士として扱う。
	} else if (lengthA <= epsilon) {
		parameterB = std::clamp(projectionB / lengthB, 0.0f, 1.0f);
	} else if (lengthB <= epsilon) {
		parameterA = std::clamp(-projectionA / lengthA, 0.0f, 1.0f);
	} else {
		const float axesDot = axisA.Dot(axisB);
		const float denominator = lengthA * lengthB - axesDot * axesDot;
		if (denominator > epsilon * lengthA * lengthB) {
			parameterA = std::clamp((axesDot * projectionB - projectionA * lengthB) / denominator, 0.0f, 1.0f);
		}
		parameterB = (axesDot * parameterA + projectionB) / lengthB;
		if (parameterB < 0.0f) {
			parameterB = 0.0f;
			parameterA = std::clamp(-projectionA / lengthA, 0.0f, 1.0f);
		} else if (parameterB > 1.0f) {
			parameterB = 1.0f;
			parameterA = std::clamp((axesDot - projectionA) / lengthA, 0.0f, 1.0f);
		}
	}
	const Vector3 separation = offset + axisA * parameterA - axisB * parameterB;
	const float radii = capsuleA.radius + capsuleB.radius;
	const float distanceSquared = separation.LengthSquared();
	if (!std::isfinite(distanceSquared) || !std::isfinite(radii)
		|| distanceSquared > radii * radii) return result;
	const float distance = std::sqrt(distanceSquared);
	result.hit = true;
	result.penetration = std::max(radii - distance, 0.0f);
	if (distanceSquared > epsilon) {
		result.normal = separation / distance;
	} else {
		// 中心軸が重なる場合も、有限で長さ1の補正方向を返す。
		const auto centerOffset = (capsuleA.p0 + capsuleA.p1 - capsuleB.p0 - capsuleB.p1) * 0.5f;
		const float centerDistance = centerOffset.Length();
		result.normal = std::isfinite(centerDistance) && centerDistance > 1e-4f
			? centerOffset / centerDistance : Vector3{1.0f, 0.0f, 0.0f};
	}
	return result;
}

SpherePushResult TestSphereTriangle(const Vector3& center, float radius, const TriangleCollider& triangle) {
	SpherePushResult result;
	Vector3 closest = ClosestPointOnTriangle(center, triangle.v[0], triangle.v[1], triangle.v[2]);
	Vector3 diff = center - closest;
	float dist = diff.Length();
	if (dist < radius) {
		result.hit = true;
		result.penetration = radius - dist;
		result.normal = (dist > 1e-6f) ? diff / dist : triangle.normal;
	}
	return result;
}

RayTriangleHit IntersectRayTriangle(const Vector3& origin, const Vector3& direction,
	float maxDistance, const TriangleCollider& triangle) {
	RayTriangleHit result;
	if (maxDistance < 0.0f || direction.LengthSquared() < 1e-12f) return result;

	// Moller–Trambore交差判定。三角形のどちらの面も許容する。
	constexpr float epsilon = 1e-6f;
	const Vector3 edge1 = triangle.v[1] - triangle.v[0];
	const Vector3 edge2 = triangle.v[2] - triangle.v[0];
	const Vector3 p = direction.Cross(edge2);
	const float determinant = edge1.Dot(p);
	if (std::abs(determinant) < epsilon) return result;

	const float inverseDeterminant = 1.0f / determinant;
	const Vector3 fromVertex = origin - triangle.v[0];
	const float u = fromVertex.Dot(p) * inverseDeterminant;
	if (u < 0.0f || u > 1.0f) return result;

	const Vector3 q = fromVertex.Cross(edge1);
	const float v = direction.Dot(q) * inverseDeterminant;
	if (v < 0.0f || u + v > 1.0f) return result;

	const float distance = edge2.Dot(q) * inverseDeterminant;
	if (distance < 0.0f || distance > maxDistance) return result;

	result.hit = true;
	result.distance = distance;
	result.position = origin + direction * distance;
	result.normal = triangle.normal;
	if (result.normal.LengthSquared() < epsilon) {
		result.normal = edge1.Cross(edge2).Normalize();
	} else {
		result.normal = result.normal.Normalize();
	}
	if (result.normal.Dot(direction) > 0.0f) result.normal = -result.normal;
	return result;
}

RayTriangleHit IntersectRayBox(const Vector3& origin, const Vector3& direction,
	float maxDistance, const Vector3& center, const Vector3& halfExtents,
	const Quaternion& rotation) {
	RayTriangleHit result;
	if (maxDistance < 0.0f || direction.LengthSquared() < 1e-12f) return result;

	Quaternion inverseRotation = rotation;
	inverseRotation.Inverse();
	const Vector3 localOrigin = inverseRotation.RotateVector(origin - center);
	const Vector3 localDirection = inverseRotation.RotateVector(direction);
	float enterDistance = 0.0f;
	float exitDistance = maxDistance;
	Vector3 enterNormal = Vector3::ZERO;
	const float origins[3] = { localOrigin.x, localOrigin.y, localOrigin.z };
	const float directions[3] = { localDirection.x, localDirection.y, localDirection.z };
	const float extents[3] = { std::abs(halfExtents.x), std::abs(halfExtents.y), std::abs(halfExtents.z) };
	for (int axis = 0; axis < 3; ++axis) {
		if (std::abs(directions[axis]) < 1e-7f) {
			if (origins[axis] < -extents[axis] || origins[axis] > extents[axis]) return result;
			continue;
		}

		float nearDistance = (-extents[axis] - origins[axis]) / directions[axis];
		float farDistance = (extents[axis] - origins[axis]) / directions[axis];
		float nearSign = -1.0f;
		if (nearDistance > farDistance) {
			std::swap(nearDistance, farDistance);
			nearSign = 1.0f;
		}
		if (nearDistance > enterDistance) {
			enterDistance = nearDistance;
			enterNormal = Vector3::ZERO;
			if (axis == 0) enterNormal.x = nearSign;
			if (axis == 1) enterNormal.y = nearSign;
			if (axis == 2) enterNormal.z = nearSign;
		}
		exitDistance = std::min(exitDistance, farDistance);
		if (enterDistance > exitDistance) return result;
	}

	// レイ開始点がボックス内部にある場合、下面を地面として誤検出しない。
	if (enterDistance <= 1e-6f || enterDistance > maxDistance || enterNormal.LengthSquared() == 0.0f) return result;
	result.hit = true;
	result.distance = enterDistance;
	result.position = origin + direction * enterDistance;
	result.normal = rotation.RotateVector(enterNormal).Normalize();
	if (result.normal.Dot(direction) > 0.0f) result.normal = -result.normal;
	return result;
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

CapsuleOBBCollision TestCapsuleOBB(const Transform* capsuleTransform, const Math::CapsuleCollider* capsule, const Transform* obbTransform, const Math::OBBCollider* obb, ECS::Registry& registry) {
	CapsuleOBBCollision result;

	// ワールド座標でのカプセルとOBBを取得
	// ※ OBBWorld は center / rotation(Quaternion) / halfExtents を持つ想定
	CapsuleWorld cap = GetWorldCapsule(capsuleTransform, capsule, registry);
	OBBWorld box = GetWorldOBB(obbTransform, obb, registry);

	// OBBのローカル空間へ変換すれば、AABB版と同じアルゴリズムがそのまま使える
	Quaternion invRot = box.rotation;
	invRot.Inverse();
	Vector3 localP0 = invRot.RotateVector(cap.p0 - box.center);
	Vector3 localP1 = invRot.RotateVector(cap.p1 - box.center);
	
	// カプセルの中心軸を表すベクトル（ローカル空間）
	Vector3 segment = localP1 - localP0;
	// ローカル空間ではOBBの中心は原点
	Vector3 boxCenter = Vector3::ZERO;

	// 原点からカプセル下端へのベクトル
	Vector3 p0ToCenter = boxCenter - localP0;

	float segmentLengthSquared = segment.LengthSquared();

	// 線分への射影でtを求める
	float t = 0.0f;
	if (segmentLengthSquared > 0.0f) {
		t = p0ToCenter.Dot(segment) / segmentLengthSquared;
	}
	// 線分範囲へclamp
	t = std::clamp(t, 0.0f, 1.0f);

	// 線分上の最近接点（ローカル空間）
	Vector3 pointOnSegmentLocal = localP0 + segment * t;
	// ローカル空間のハーフエクステントにclampして最近接点を求める
	Vector3 closestOnBoxLocal = Clamp(pointOnSegmentLocal, -box.halfExtents, box.halfExtents);

	// 差分ベクトル（ローカル空間）
	Vector3 diffLocal = pointOnSegmentLocal - closestOnBoxLocal;

	// 0除算を避けるために1e-8fでclamp
	float dist = std::max(diffLocal.Length(), 1e-8f);
	float r = cap.radius;

	// 衝突判定(最近接点間の距離が半径より小さいか)
	if (dist <= r) {
		// 衝突している
		result.hit = true;
		// 貫通量
		result.penetration = r - dist;

		// 法線・最近接点をワールド空間へ戻す
		Vector3 normalLocal = diffLocal / dist;
		result.normal = box.rotation.RotateVector(normalLocal);
		result.closestOnBox = box.center + box.rotation.RotateVector(closestOnBoxLocal);
		result.closestOnCapsule = box.center + box.rotation.RotateVector(pointOnSegmentLocal);
	}

	return result;
}

CapsuleTriangleCollision TestCapsuleTriangle(
	const Transform* capsuleTransform,
	const Math::CapsuleCollider* capsule,
	const Math::TriangleCollider& triangle, ECS::Registry& registry) {
	CapsuleWorld cap = GetWorldCapsule(capsuleTransform, capsule, registry);
	// 静止状態の接触にCCDスキンを含めることで、スイープ停止時にも接地状態が維持されるようにする。
	auto result = TestSegmentTriangle(cap.p0, cap.p1, cap.radius + 0.001f, triangle);
	result.penetration = std::max(0.0f, result.penetration - 0.001f);
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
	// 端点が一致したカプセルは球として扱い、ゼロ除算を防ぐ。
	float t = l > 1e-12f ? w.Dot(v) / l : 0.0f;

	// 線分の範囲内にクランプ
	t = std::clamp(t, 0.0f, 1.0f);

	//  線分上の最近接点を求める
	Vector3 nearPoint = cap.p0 + v * t;

	// 距離を算出
	Vector3 delta = nearPoint - sph.center;
	float distanceSquared = delta.LengthSquared();
	float radiusSum = cap.radius + sph.radius;

	// 半径の合計以下かどうかを返す
	result.hit = distanceSquared <= radiusSum * radiusSum;
	if (!result.hit) return result;

	float distance = std::sqrt(distanceSquared);
	if (distanceSquared > 1e-12f) {
		result.normal = delta / distance;
	} else if (l > 1e-12f) {
		// 中心が軸上にある場合は、軸に垂直な方向へ押し出す。
		Vector3 axis = v / std::sqrt(l);
		Vector3 reference = std::abs(axis.x) < 0.9f ? Vector3::RIGHT : Vector3::UP;
		result.normal = axis.Cross(reference).Normalize();
	} else {
		// 球同士の中心も一致した場合は、一定の方向を選ぶ。
		result.normal = Vector3::RIGHT;
	}
	result.penetration = std::max(radiusSum - distance, 0.0f);
	result.closestOnCapsule = nearPoint;
	result.closestOnSphere = sph.center + result.normal * sph.radius;

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
