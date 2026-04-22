#pragma once
#include "../Types/Vector3.h"

namespace NoEngine {
namespace Math {

/// <summary>
/// 軸に平行な境界ボックス (AABB) を表すコライダ構造体。
/// </summary>
struct AABBCollider {
	Math::Vector3 min;
	Math::Vector3 max;
};

/// <summary>
/// カプセル形状のコライダーを表す構造体。ローカル空間での端点2つと半径を保持する。
/// </summary>
struct CapsuleCollider {
	Math::Vector3 localP0;
	Math::Vector3 localP1;
	float radius;
};

}
namespace Component {
/// <summary>
/// オブジェクトの接地状態と地面の高さを表す構造体。
/// </summary>
struct GroundStateComponent {
	bool isGrounded = false;
	float groundHeight = 0.f;	// 接地している場合の地面の高さ
};

/// <summary>
/// 押し戻しの種別を表す列挙型。
/// </summary>
enum class BodyType {
	Dynamic,   // 押し戻される
	Static	   // 押し戻されない
};

/// <summary>
/// 押し戻しの種別を表す構造体。オブジェクトが動的か静的かを示す。
/// </summary>
struct CollisionBody {
	BodyType type;
};
}
}