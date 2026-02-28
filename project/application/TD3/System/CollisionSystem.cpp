#include "CollisionSystem.h"
#include "engine/Math/Types/Calculations/Vector3Calculations.h"
#include "../Component/ColliderComponent.h"
#include "../Component/ShockwaveComponent.h"
#include "engine/Functions/Renderer/Primitive.h"
#include <algorithm>

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

using namespace NoEngine::MathCalculations;

// ============================================================
//  Update
// ============================================================

void CollisionSystem::Update(No::Registry& registry, float deltaTime) {
	(void)deltaTime;

	// 1. 全コライダーのワールド座標と半径を更新（描画はここでは行わない）
	UpdateCollider(registry);

	// 2. 球体同士の衝突判定（総当たり）
	auto sphereView = registry.View<SphereColliderComponent>();

	for (auto entity1 : sphereView) {
		if (!registry.Has<SphereColliderComponent>(entity1)) continue;
		auto* a = registry.GetComponent<SphereColliderComponent>(entity1);

		for (auto entity2 : sphereView) {
			// 自分自身はスキップ
			if (entity1 == entity2 || !registry.Has<SphereColliderComponent>(entity2)) continue;

			auto* b = registry.GetComponent<SphereColliderComponent>(entity2);

			// マスクフィルタリング（双方向チェック）
			if ((a->colliderType & b->collideMask) == 0 ||
				(b->colliderType & a->collideMask) == 0) continue;

			// 距離判定
			if (CheckSphereToSphere(a->center, b->center, a->worldRadius, b->worldRadius)) {
				a->isCollied = true;
				b->isCollied = true;
				a->colliedWith = static_cast<ColliderMask>(b->colliderType);
				b->colliedWith = static_cast<ColliderMask>(a->colliderType);
				a->colliedEntity = entity2;
				b->colliedEntity = entity1;
			}
		}
	}

	// デバッグ描画（衝突判定完了後に実行することで isCollied が正しい状態になる）
	// ShockwaveComponent を持つエンティティは ShockwaveSystem が独自に描画するためスキップする
#ifdef USE_IMGUI
	auto drawView = registry.View<SphereColliderComponent>();
	for (auto entity : drawView) {
		if (!registry.Has<SphereColliderComponent>(entity)) continue;

		// 衝撃波は ShockwaveSystem が独自に描画するので CollisionSystem では描画しない
		if (registry.Has<ShockwaveComponent>(entity)) continue;

		auto* sphere = registry.GetComponent<SphereColliderComponent>(entity);

		No::Vector3 color = sphere->isCollied
			? No::Vector3{ 1.0f, 0.0f, 0.0f }  // 衝突中は赤
		: No::Vector3{ 0.0f, 1.0f, 0.0f };  // 通常は緑

		//一時的にコメントアウト

		//NoEngine::Primitive::DrawSphere(sphere->center, sphere->worldRadius,
		//	{ color.x, color.y, color.z, 0.5f });
	}
#endif
}

// ============================================================
//  UpdateCollider
// ============================================================

void CollisionSystem::UpdateCollider(No::Registry& registry) {
	auto sphereView = registry.View<SphereColliderComponent, No::TransformComponent>();

	// 球状コライダー更新
	for (auto entity : sphereView) {
		if (!registry.Has<SphereColliderComponent>(entity)) continue;

		auto* sphereA = registry.GetComponent<SphereColliderComponent>(entity);
		auto* transformA = registry.GetComponent<No::TransformComponent>(entity);

		// ワールド座標を更新
		sphereA->center = transformA->translate;

		// スケールの最大値を取得（非等方スケール対応）
		float maxScale = std::max({ std::abs(transformA->scale.x),
								   std::abs(transformA->scale.y),
								   std::abs(transformA->scale.z) });

		// ワールド空間での半径を計算
		sphereA->worldRadius = sphereA->radius * maxScale;

		// 衝突フラグをリセット
		sphereA->isCollied = false;
	}
}

// ============================================================
//  CheckSphereToSphere
// ============================================================

bool CollisionSystem::CheckSphereToSphere(const No::Vector3& center1, const No::Vector3& center2,
	const float radius1, const float radius2) {
	No::Vector3 diff = center1 - center2;
	float distanceSq = LengthSquared(diff);
	float radiusSum = radius1 + radius2;
	return distanceSq <= radiusSum * radiusSum;
}

// ============================================================
//  CheckBoxToBox（将来の拡張用）
// ============================================================

bool CollisionSystem::CheckBoxToBox(const No::Vector3& center1, const No::Vector3& center2,
	const No::Vector3& size1, const No::Vector3& size2) {
	auto minA = center1 - size1 * 0.5f;
	auto maxA = center1 + size1 * 0.5f;

	auto minB = center2 - size2 * 0.5f;
	auto maxB = center2 + size2 * 0.5f;

	return minA.x <= maxB.x && maxA.x >= minB.x &&
		minA.y <= maxB.y && maxA.y >= minB.y &&
		minA.z <= maxB.z && maxA.z >= minB.z;
}

// ============================================================
//  CheckBoxToSphere（将来の拡張用）
// ============================================================

bool CollisionSystem::CheckBoxToSphere(const No::Vector3& center1, const No::Vector3& center2,
	const No::Vector3& size, const float radius) {
	auto min = center1 - size * 0.5f;
	auto max = center1 + size * 0.5f;

	// AABBと球の最近接点を求める
	No::Vector3 closestPoint = {
		std::clamp(center2.x, min.x, max.x),
		std::clamp(center2.y, min.y, max.y),
		std::clamp(center2.z, min.z, max.z)
	};

	// 最近接点と球の中心との距離を求める
	float distance = LengthSquared(closestPoint - center2);

	return distance <= radius * radius;
}