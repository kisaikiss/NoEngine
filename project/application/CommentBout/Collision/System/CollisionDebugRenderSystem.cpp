#include "stdafx.h"
#include "CollisionDebugRenderSystem.h"
#include "application/CommentBout/Collision/Component/CollisionDebugConfigComponent.h"
#include "application/CommentBout/Collision/Component/Collider3DComponent.h"
#include "application/CommentBout/Collision/Component/Collider2DComponent.h"
#include "application/CommentBout/Component/PlayerHitboxComponent.h"
#include "application/CommentBout/Component/FieldObject/FieldPlacementComponent.h"
#include "application/CommentBout/GameTag.h"
#include "engine/Functions/ECS/Component/CameraComponent.h"
#include "engine/Functions/ECS/Component/Transform2DComponent.h"
#include "engine/Functions/Renderer/Primitive.h"
#include <algorithm>
#include <cmath>

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

namespace {
No::Vector3 NormalizeOrDefault(const No::Vector3& v, const No::Vector3& fallback) {
	if (v.LengthSquared() <= 0.000001f) {
		return fallback;
	}
	return v.Normalize();
}

No::Color MakeColliderColor(bool isColliding) {
	return isColliding ? No::Color(1.0f, 0.2f, 0.2f, 1.0f) : No::Color(0.2f, 1.0f, 0.2f, 1.0f);
}
}

namespace CommentBoutCollision {

CollisionDebugConfigComponent* CollisionDebugRenderSystem::EnsureConfig(No::Registry& registry) {
	for (auto entity : registry.View<CollisionDebugConfigTag, CollisionDebugConfigComponent>()) {
		auto* config = registry.GetComponent<CollisionDebugConfigComponent>(entity);
		if (config) {
			return config;
		}
	}

	auto entity = registry.GenerateEntity();
	registry.AddComponent<CollisionDebugConfigTag>(entity);
	return registry.AddComponent<CollisionDebugConfigComponent>(entity);
}

void CollisionDebugRenderSystem::DrawConfigImGui(CollisionDebugConfigComponent& config) {
#ifdef USE_IMGUI
	ImGui::Begin("Collision Debug");
	ImGui::Checkbox("衝突デバッグ表示", &config.enableCollisionDebug);
	if (config.enableCollisionDebug) {
		ImGui::Checkbox("敵コライダー", &config.showEnemyCollider);
		ImGui::Checkbox("フィールドコライダー", &config.showFieldCollider);
		ImGui::Checkbox("自機ヒットボックス(2D)", &config.showPlayerHitboxOverlay);
		ImGui::Checkbox("カメラゲート", &config.showCameraGateWire);
		ImGui::Checkbox("攻撃コライダー", &config.showPlayerAttackCollider);
	}
	ImGui::End();
#else
	static_cast<void>(config);
#endif
}

void CollisionDebugRenderSystem::DrawEnemyColliderDebug(No::Registry& registry, const CollisionDebugConfigComponent& config) {
	if (!config.enableCollisionDebug || !config.showEnemyCollider) {
		return;
	}

	for (auto entity : registry.View<CBRailEnemyTag, Collider3DComponent>()) {
		auto* collider = registry.GetComponent<Collider3DComponent>(entity);
		if (!collider) {
			continue;
		}

		const No::Color color = MakeColliderColor(collider->isColliding);
		if (collider->shapeType == ShapeType3D::Box) {
			NoEngine::Primitive::DrawCube(collider->worldPosition, collider->worldBoxSize, color);
		} else {
			NoEngine::Primitive::DrawSphere(collider->worldPosition, collider->worldRadius, color, 10, 10);
		}
	}
}

void CollisionDebugRenderSystem::DrawFieldColliderDebug(No::Registry& registry, const CollisionDebugConfigComponent& config) {
	if (!config.enableCollisionDebug || !config.showFieldCollider) {
		return;
	}

	for (auto entity : registry.View<CBFieldObjectTag, FieldPlacementComponent, Collider3DComponent>()) {
		auto* placement = registry.GetComponent<FieldPlacementComponent>(entity);
		auto* collider = registry.GetComponent<Collider3DComponent>(entity);
		if (!placement || !collider || !placement->hasCollision) {
			continue;
		}

		NoEngine::Primitive::DrawCube(collider->worldPosition, collider->worldBoxSize, MakeColliderColor(collider->isColliding));
	}
}

void CollisionDebugRenderSystem::DrawPlayerHitboxDebug(No::Registry& registry, const CollisionDebugConfigComponent& config) {
	if (!config.enableCollisionDebug || !config.showPlayerHitboxOverlay) {
		return;
	}

#ifdef USE_IMGUI
	ImDrawList* drawList = ImGui::GetForegroundDrawList();
	if (!drawList) {
		return;
	}

	for (auto hitboxEntity : registry.View<CBPlayerHitboxTag, PlayerHitboxComponent>()) {
		auto* hitbox = registry.GetComponent<PlayerHitboxComponent>(hitboxEntity);
		if (!hitbox || hitbox->playerEntity == No::nullEntity) {
			continue;
		}

		// 自機の実際の 2D コライダー bounds を描画
		auto* col2D = registry.GetComponent<CommentBoutCollision::Collider2DComponent>(hitbox->playerEntity);
		if (col2D) {
			const No::Vector2 half = col2D->worldSize * 0.5f;
			const ImVec2 pMin(col2D->screenPosition.x - half.x, col2D->screenPosition.y - half.y);
			const ImVec2 pMax(col2D->screenPosition.x + half.x, col2D->screenPosition.y + half.y);
			drawList->AddRect(pMin, pMax, IM_COL32(64, 255, 64, 255), 0.0f, 0, 2.0f);
		}
	}
#else
	static_cast<void>(registry);
#endif
}

void CollisionDebugRenderSystem::DrawCameraGateDebug(No::Registry& registry, const CollisionDebugConfigComponent& config) {
	if (!config.enableCollisionDebug || !config.showCameraGateWire) {
		return;
	}

	No::TransformComponent* activeCameraTransform = nullptr;
	auto cameraView = registry.View<No::CameraComponent, No::TransformComponent, No::ActiveCameraTag>();
	auto cameraIt = cameraView.begin();
	if (cameraIt != cameraView.end()) {
		activeCameraTransform = registry.GetComponent<No::TransformComponent>(*cameraIt);
	}
	if (!activeCameraTransform) {
		return;
	}

	for (auto hitboxEntity : registry.View<CBPlayerHitboxTag, PlayerHitboxComponent>()) {
		auto* hitbox = registry.GetComponent<PlayerHitboxComponent>(hitboxEntity);
		if (!hitbox || !hitbox->useCameraGateForPlayerHit) {
			continue;
		}

		No::Matrix4x4 cameraWorld = activeCameraTransform->MakeAffineMatrix4x4();
		const No::Vector3 cameraPos = activeCameraTransform->GetWorldPosition();
		No::Vector3 right = NormalizeOrDefault(cameraWorld.TransformNormal(No::Vector3::RIGHT), No::Vector3::RIGHT);
		No::Vector3 up = NormalizeOrDefault(cameraWorld.TransformNormal(No::Vector3::UP), No::Vector3::UP);
		No::Vector3 forward = NormalizeOrDefault(cameraWorld.TransformNormal(No::Vector3::FORWARD), No::Vector3::FORWARD);

		const float nearZ = std::max(0.0f, hitbox->cameraGateNear);
		const float depth = std::max(0.001f, hitbox->cameraGateDepth);
		const float farZ = nearZ + depth;
		const float halfW = std::max(0.001f, hitbox->cameraGateHalfWidth);
		const float halfH = std::max(0.001f, hitbox->cameraGateHalfHeight);

		const No::Vector3 nearCenter = cameraPos + forward * nearZ;
		const No::Vector3 farCenter = cameraPos + forward * farZ;

		const No::Vector3 n0 = nearCenter - right * halfW - up * halfH;
		const No::Vector3 n1 = nearCenter + right * halfW - up * halfH;
		const No::Vector3 n2 = nearCenter + right * halfW + up * halfH;
		const No::Vector3 n3 = nearCenter - right * halfW + up * halfH;

		const No::Vector3 f0 = farCenter - right * halfW - up * halfH;
		const No::Vector3 f1 = farCenter + right * halfW - up * halfH;
		const No::Vector3 f2 = farCenter + right * halfW + up * halfH;
		const No::Vector3 f3 = farCenter - right * halfW + up * halfH;

		const No::Color gateColor(1.0f, 1.0f, 0.2f, 1.0f);
		NoEngine::Primitive::DrawLine(n0, n1, gateColor);
		NoEngine::Primitive::DrawLine(n1, n2, gateColor);
		NoEngine::Primitive::DrawLine(n2, n3, gateColor);
		NoEngine::Primitive::DrawLine(n3, n0, gateColor);
		NoEngine::Primitive::DrawLine(f0, f1, gateColor);
		NoEngine::Primitive::DrawLine(f1, f2, gateColor);
		NoEngine::Primitive::DrawLine(f2, f3, gateColor);
		NoEngine::Primitive::DrawLine(f3, f0, gateColor);
		NoEngine::Primitive::DrawLine(n0, f0, gateColor);
		NoEngine::Primitive::DrawLine(n1, f1, gateColor);
		NoEngine::Primitive::DrawLine(n2, f2, gateColor);
		NoEngine::Primitive::DrawLine(n3, f3, gateColor);
	}
}

void CollisionDebugRenderSystem::DrawPlayerAttackColliderDebug(No::Registry& registry, const CollisionDebugConfigComponent& config) {
	if (!config.enableCollisionDebug || !config.showPlayerAttackCollider) {
		return;
	}

#ifdef USE_IMGUI
	ImDrawList* drawList = ImGui::GetForegroundDrawList();
	if (!drawList) {
		return;
	}

	for (auto entity : registry.View<CBAttackEffectTag, CommentBoutCollision::Collider2DComponent>()) {
		auto* collider = registry.GetComponent<CommentBoutCollision::Collider2DComponent>(entity);
		if (!collider) {
			continue;
		}

		const No::Vector2 half = collider->worldSize * 0.5f;
		const ImVec2 pMin(collider->screenPosition.x - half.x, collider->screenPosition.y - half.y);
		const ImVec2 pMax(collider->screenPosition.x + half.x, collider->screenPosition.y + half.y);
		const ImU32 color = collider->isColliding
			? IM_COL32(255, 64, 64, 255)
			: IM_COL32(255, 220, 64, 255);
		drawList->AddRect(pMin, pMax, color, 0.0f, 0, 2.0f);
	}
#else
	static_cast<void>(registry);
#endif
}

void CollisionDebugRenderSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);

	auto* config = EnsureConfig(registry);
	if (!config) {
		return;
	}

	DrawConfigImGui(*config);
	DrawEnemyColliderDebug(registry, *config);
	DrawFieldColliderDebug(registry, *config);
	DrawPlayerHitboxDebug(registry, *config);
	DrawPlayerAttackColliderDebug(registry, *config);
	DrawCameraGateDebug(registry, *config);
}

}
