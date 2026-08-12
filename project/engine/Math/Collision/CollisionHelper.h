#pragma once
#include "../Types/Transform.h"
#include "../Types/Transform2D.h"

#include "ColliderComponent.h"
#include "engine/Functions/ECS/Registry.h"

namespace NoEngine {
struct AABBWorld {
	Math::Vector3 min;
	Math::Vector3 max;
};

struct OBBWorld {
	Math::Vector3 center;
	Math::Quaternion rotation;
	Math::Vector3 halfExtents;
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

struct SphereWorld {
	Math::Vector3 center;
	float radius;
};

/// <summary>
/// AABBコライダーをワールド座標へ直す
/// </summary>
/// <param name="transform">トランスフォーム</param>
/// <param name="aabb">AABBコライダー</param>
/// <returns>ワールド座標でのAABBコライダー</returns>
AABBWorld GetWorldAABB(const Transform* transform, const Math::AABBCollider* aabb, ECS::Registry& registry);

/// <summary>
/// OBBコライダーをワールド座標へ直す
/// </summary>
/// <param name="transform">トランスフォーム</param>
/// <param name="obb">OBBコライダー</param>
/// <returns>ワールド座標でのOBBコライダー</returns>
OBBWorld GetWorldOBB(const Transform* transform, const Math::OBBCollider* obb, ECS::Registry& registry);

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
CapsuleWorld GetWorldCapsule(const Transform* transform, const Math::CapsuleCollider* capsule, ECS::Registry& registry);

/// <summary>
/// スフィアコライダーをワールド座標へ直す
/// </summary>
/// <param name="transform">トランスフォーム</param>
/// <param name="sphere">球コライダー</param>
/// <returns>ワールド座標での球コライダー</returns>
SphereWorld GetWorldSphere(const Transform* transform, const Math::SphereCollider* sphere, ECS::Registry& registry);

/// <summary>
/// カプセルコライダーをワールド座標上のAABBへ変換する
/// </summary>
/// <param name="transform">トランスフォーム</param>
/// <param name="capsule">カプセルコライダー</param>
/// <returns>ワールド座標上のAABB</returns>
Math::AABBCollider ComputeCapsuleWorldBounds(const Transform* transform, const Math::CapsuleCollider* capsule, ECS::Registry& registry);
}
