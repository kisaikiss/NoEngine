#pragma once
#include "../Types/Vector3.h"
#include "../Types/Vector2.h"
#include "engine/Functions/ECS/Entity.h"

namespace NoEngine {
namespace Math {

/// <summary>
/// 軸に平行な境界ボックス (AABB) を表すコライダ構造体。
/// </summary>
struct AABBCollider {
	Vector3 min{ FLT_MAX,  FLT_MAX,  FLT_MAX };
	Vector3 max{ -FLT_MAX, -FLT_MAX, -FLT_MAX };

	void Expand(const Math::Vector3& p) {
		min.x = std::min(min.x, p.x);
		min.y = std::min(min.y, p.y);
		min.z = std::min(min.z, p.z);
		max.x = std::max(max.x, p.x);
		max.y = std::max(max.y, p.y);
		max.z = std::max(max.z, p.z);
	}

	void Expand(const AABBCollider& other) {
		Expand(other.min);
		Expand(other.max);
	}

	// 表面積（SAH の分割基準に使用）
	float SurfaceArea() const {
		float dx = max.x - min.x;
		float dy = max.y - min.y;
		float dz = max.z - min.z;
		return 2.f * (dx * dy + dy * dz + dz * dx);
	}

	// 別の AABB と重なるか
	bool Overlaps(const AABBCollider& o) const {
		return min.x <= o.max.x && max.x >= o.min.x
			&& min.y <= o.max.y && max.y >= o.min.y
			&& min.z <= o.max.z && max.z >= o.min.z;
	}

	// 点を含むか
	bool Contains(const Math::Vector3& p) const {
		return p.x >= min.x && p.x <= max.x
			&& p.y >= min.y && p.y <= max.y
			&& p.z >= min.z && p.z <= max.z;
	}
};

struct OBBCollider {
	Vector3 extents;
};


/// <summary>
/// 2D の軸整列境界ボックス (AABB) を表すコライダー構造体。衝突判定や領域表現に使用される。
/// </summary>
struct AABBCollider2D {
	Math::Vector2 min;
	Math::Vector2 max;
};

/// <summary>
/// カプセル形状のコライダーを表す構造体。ローカル空間での端点2つと半径を保持する。
/// </summary>
struct CapsuleCollider {
	Math::Vector3 localP0;
	Math::Vector3 localP1;
	float radius;
};

/// <summary>
/// 球形状のコライダーを表す構造体。
/// </summary>
struct SphereCollider {
	Math::Vector3 localCenter;	// ローカル空間での中心点
	float radius = 0.5f;		// 球の半径
};

/// <summary>
/// 三角形状のコライダーを表す構造体
/// </summary>
struct TriangleCollider {
	std::array<Math::Vector3, 3> v;   // 頂点
	Math::Vector3 normal;			  // 面法線（正規化済み）

	// 重心を返す
	Math::Vector3 Centroid() const {
		return {
			(v[0].x + v[1].x + v[2].x) / 3.f,
			(v[0].y + v[1].y + v[2].y) / 3.f,
			(v[0].z + v[1].z + v[2].z) / 3.f
		};
	}
};

}
namespace Component {
/// <summary>
/// オブジェクトの接地状態と地面の高さを表す構造体。
/// </summary>
struct GroundStateComponent {
	bool isGrounded = false;
	bool preIsGrounded = false;
	float groundHeight = 0.f;	// 接地している場合の地面の高さ
	ECS::Entity groundEntity = ECS::INVALID_ENTITY;
};

/// <summary>
/// 足場の移動量を保持する構造体
/// </summary>
struct PlatformDeltaComponent {
	Math::Vector3 previousTranslate = Math::Vector3::ZERO;
	Math::Vector3 frameDelta = Math::Vector3::ZERO;
};

/// <summary>
/// 押し戻しの種別を表す列挙型。
/// </summary>
enum class BodyType {
	Dynamic,   // 押し戻される
	Static,	   // 押し戻されない
	Through,   // スルーされる(相手も押し戻されない)
};

/// <summary>
/// 押し戻しの種別を表す構造体。オブジェクトが動的か静的かを示す。
/// </summary>
struct CollisionBody {
	BodyType type;
};
}
}