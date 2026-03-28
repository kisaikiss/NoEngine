#pragma once
#include "engine/NoEngine.h"
#include "application/CommentBout/Collision/Utility/CollisionMask.h"

namespace CommentBoutCollision {

	/// <summary>
	/// 衝突イベントの種別。
	/// 判定方式ごとの分岐をResolver側で行えるようにする。
	/// </summary>
	enum class CollisionEventType {
		Collider3DVs3D,
		Projected3DVs2D
	};

	/// <summary>
	/// CollisionSystem が1フレーム中に検出した衝突ペア情報。
	/// Entityイベント化しておくことで、複数Systemが同じ衝突結果を参照できる。
	/// </summary>
	struct CollisionEventComponent {
		CollisionEventType type = CollisionEventType::Collider3DVs3D;
		No::Entity entityA = No::nullEntity;
		No::Entity entityB = No::nullEntity;
		CollisionType layerA = CollisionType::None;
		CollisionType layerB = CollisionType::None;
	};

	/// <summary>
	/// 衝突イベントEntityを識別するタグ。
	/// CollisionSystemの先頭で前フレーム分を掃除するために使用する。
	/// </summary>
	struct CollisionEventTag {};
}
