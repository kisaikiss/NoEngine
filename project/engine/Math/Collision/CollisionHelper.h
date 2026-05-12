#pragma once
#include "../Types/Transform.h"
#include "../Types/Transform2D.h"

#include "ColliderComponent.h"

namespace NoEngine {
struct AABBWorld {
	Math::Vector3 min;
	Math::Vector3 max;
};

struct AABBWorld2D {
	Math::Vector2 min;
	Math::Vector2 max;
};

struct CapsuleWorld {
	Math::Vector3 p0;
	Math::Vector3 p1;
	float radius;
};

/// <summary>
/// AABBコライダーをワールド座標へ直す
/// </summary>
/// <param name="transform">トランスフォーム</param>
/// <param name="aabb">AABBコライダー</param>
/// <returns>ワールド座標でのAABBコライダー</returns>
AABBWorld GetWorldAABB(const Transform* transform, const Math::AABBCollider* aabb);

/// <summary>
/// AABBコライダーをワールド座標へ直す
/// </summary>
/// <param name="transform">トランスフォーム</param>
/// <param name="aabb">AABBコライダー</param>
/// <returns>ワールド座標でのAABBコライダー</returns>
AABBWorld2D GetWorldAABB2D(const Transform2D* transform, const Math::AABBCollider2D* aabb);

/// <summary>
/// カプセルコライダーをワールド座標へ直す
/// </summary>
/// <param name="transform">トランスフォーム</param>
/// <param name="capsule">カプセルコライダー</param>
/// <returns>ワールド座標でのカプセルコライダー</returns>
CapsuleWorld GetWorldCapsule(const Transform* transform, const Math::CapsuleCollider* capsule);
}
