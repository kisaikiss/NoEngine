#include "stdafx.h"
#include "EnemyVisualSystem.h"
#include "application/CommentBout/Component/EnemyComponent.h"
#include "application/CommentBout/Component/HealthComponent.h"
#include "application/CommentBout/Component/PlayerHitboxComponent.h"
#include "application/CommentBout/Component/EnemyRewardSourceComponent.h"
#include "application/CommentBout/Component/EnemyRewardOrbComponent.h"
#include "application/CommentBout/Component/HpBarComponent.h"
#include "application/CommentBout/Component/DamageFlashComponent.h"
#include "application/CommentBout/Component/GameResourceComponent.h"
#include "application/CommentBout/Collision/Component/Collider3DComponent.h"
#include "application/CommentBout/Collision/Component/ProjectedColliderComponent.h"
#include "application/CommentBout/Collision/Utility/CollisionAlgorithms.h"
#include "application/CommentBout/Collision/Utility/CoordinateConverter.h"
#include "application/CommentBout/GameTag.h"
#include "engine/Functions/ECS/Component/CameraComponent.h"
#include "engine/Functions/ECS/Component/Transform2DComponent.h"
#include "engine/Functions/Renderer/Primitive.h"
#include <algorithm>
#include <cmath>
#include "engine/Runtime/GraphicsCore.h"

namespace No {
using ::NoEngine::Primitive;
}

namespace {
No::Vector3 NormalizeOrDefault(const No::Vector3& v, const No::Vector3& fallback) {
	if (v.LengthSquared() <= 0.000001f) {
		return fallback;
	}
	return v.Normalize();
}

enum class HitDebugState {
	None,
	InRangeHit,
	OutOfRangeHit
};

NoEngine::Math::Color ToColliderDebugColor(HitDebugState state) {
	switch (state) {
	case HitDebugState::InRangeHit:
		return NoEngine::Math::Color(1.0f, 0.0f, 0.0f, 1.0f);
	case HitDebugState::OutOfRangeHit:
		return NoEngine::Math::Color(1.0f, 1.0f, 0.0f, 1.0f);
	default:
		return NoEngine::Math::Color(1.0f, 1.0f, 1.0f, 1.0f);
	}
}

bool CheckProjectedVsPlayerSprite(
	const CommentBoutCollision::ProjectedColliderComponent& projected,
	const No::Transform2DComponent& playerTransform2D
) {
	if (!projected.isVisible) {
		return false;
	}

	if (projected.isBox) {
		return CommentBoutCollision::CollisionAlgorithms::CheckConvexHullAABB(
			projected.convexHull,
			playerTransform2D.translate,
			playerTransform2D.scale
		);
	}

	return CommentBoutCollision::CollisionAlgorithms::CheckCircleAABB(
		projected.screenPosition,
		projected.screenRadius,
		playerTransform2D.translate,
		playerTransform2D.scale
	);
}

bool CheckEnemyInCameraGate(
	const No::Vector3& enemyWorldPos,
	No::TransformComponent& cameraTransform,
	const PlayerHitboxComponent& hitbox,
	float& outCamX,
	float& outCamY,
	float& outCamZ
) {
	No::Matrix4x4 cameraWorld = cameraTransform.MakeAffineMatrix4x4();
	const No::Vector3 cameraPos = cameraTransform.GetWorldPosition();
	No::Vector3 right = NormalizeOrDefault(cameraWorld.TransformNormal(No::Vector3::RIGHT), No::Vector3::RIGHT);
	No::Vector3 up = NormalizeOrDefault(cameraWorld.TransformNormal(No::Vector3::UP), No::Vector3::UP);
	No::Vector3 forward = NormalizeOrDefault(cameraWorld.TransformNormal(No::Vector3::FORWARD), No::Vector3::FORWARD);

	const No::Vector3 toEnemy = enemyWorldPos - cameraPos;
	outCamX = toEnemy.Dot(right);
	outCamY = toEnemy.Dot(up);
	outCamZ = toEnemy.Dot(forward);

	const float nearZ = std::max(0.0f, hitbox.cameraGateNear);
	const float depth = std::max(0.001f, hitbox.cameraGateDepth);
	const float halfW = std::max(0.001f, hitbox.cameraGateHalfWidth);
	const float halfH = std::max(0.001f, hitbox.cameraGateHalfHeight);

	if (outCamZ < nearZ || outCamZ > nearZ + depth) {
		return false;
	}
	if (std::abs(outCamX) > halfW) {
		return false;
	}
	if (std::abs(outCamY) > halfH) {
		return false;
	}
	return true;
}

void DrawCameraGateDebug(No::TransformComponent& cameraTransform, const PlayerHitboxComponent& hitbox) {
	No::Matrix4x4 cameraWorld = cameraTransform.MakeAffineMatrix4x4();
	const No::Vector3 cameraPos = cameraTransform.GetWorldPosition();
	No::Vector3 right = NormalizeOrDefault(cameraWorld.TransformNormal(No::Vector3::RIGHT), No::Vector3::RIGHT);
	No::Vector3 up = NormalizeOrDefault(cameraWorld.TransformNormal(No::Vector3::UP), No::Vector3::UP);
	No::Vector3 forward = NormalizeOrDefault(cameraWorld.TransformNormal(No::Vector3::FORWARD), No::Vector3::FORWARD);

	const float nearZ = std::max(0.0f, hitbox.cameraGateNear);
	const float depth = std::max(0.001f, hitbox.cameraGateDepth);
	const float farZ = nearZ + depth;
	const float halfW = std::max(0.001f, hitbox.cameraGateHalfWidth);
	const float halfH = std::max(0.001f, hitbox.cameraGateHalfHeight);

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

	const NoEngine::Math::Color gateColor(1.0f, 1.0f, 0.2f, 1.0f);
	No::Primitive::DrawLine(n0, n1, gateColor);
	No::Primitive::DrawLine(n1, n2, gateColor);
	No::Primitive::DrawLine(n2, n3, gateColor);
	No::Primitive::DrawLine(n3, n0, gateColor);
	No::Primitive::DrawLine(f0, f1, gateColor);
	No::Primitive::DrawLine(f1, f2, gateColor);
	No::Primitive::DrawLine(f2, f3, gateColor);
	No::Primitive::DrawLine(f3, f0, gateColor);
	No::Primitive::DrawLine(n0, f0, gateColor);
	No::Primitive::DrawLine(n1, f1, gateColor);
	No::Primitive::DrawLine(n2, f2, gateColor);
	No::Primitive::DrawLine(n3, f3, gateColor);
	No::Primitive::DrawLine(n0, n2, gateColor);
	No::Primitive::DrawLine(n1, n3, gateColor);
	No::Primitive::DrawLine(f0, f2, gateColor);
	No::Primitive::DrawLine(f1, f3, gateColor);
	No::Primitive::DrawLine(cameraPos, farCenter, NoEngine::Math::Color(0.2f, 1.0f, 1.0f, 1.0f));
}

void DrawColliderDebug(const CommentBoutCollision::Collider3DComponent& collider, const NoEngine::Math::Color& color) {
	if (collider.shapeType == CommentBoutCollision::ShapeType3D::Box) {
		No::Primitive::DrawCube(collider.worldPosition, collider.worldBoxSize, color);
	} else {
		No::Primitive::DrawSphere(collider.worldPosition, collider.worldRadius, color, 10, 10);
	}
}

#ifdef USE_IMGUI
ImU32 ToOverlayDebugColor(HitDebugState state) {
	switch (state) {
	case HitDebugState::InRangeHit:
		return IM_COL32(255, 64, 64, 255);
	case HitDebugState::OutOfRangeHit:
		return IM_COL32(255, 220, 64, 255);
	default:
		return IM_COL32(255, 255, 255, 255);
	}
}

ImVec2 ToImVec2(const No::Vector2& v) {
	return ImVec2(v.x, v.y);
}

void DrawSpriteGateDebugOverlay(
	const CommentBoutCollision::ProjectedColliderComponent& projected,
	const No::Transform2DComponent& playerTransform2D,
	HitDebugState state
) {
	ImDrawList* dl = ImGui::GetForegroundDrawList();
	if (!dl) {
		return;
	}

	const ImU32 enemyColor = ToOverlayDebugColor(state);
	const ImU32 playerColor = IM_COL32(32, 255, 32, 255);

	const No::Vector2 playerHalf = playerTransform2D.scale * 0.5f;
	const ImVec2 playerMin(playerTransform2D.translate.x - playerHalf.x, playerTransform2D.translate.y - playerHalf.y);
	const ImVec2 playerMax(playerTransform2D.translate.x + playerHalf.x, playerTransform2D.translate.y + playerHalf.y);
	dl->AddRect(playerMin, playerMax, playerColor, 0.0f, 0, 2.0f);

	if (projected.isBox) {
		const size_t n = projected.convexHull.size();
		if (n >= 2) {
			for (size_t i = 0; i < n; ++i) {
				const No::Vector2& a = projected.convexHull[i];
				const No::Vector2& b = projected.convexHull[(i + 1) % n];
				dl->AddLine(ToImVec2(a), ToImVec2(b), enemyColor, 2.0f);
			}
		}
	} else {
		dl->AddCircle(ToImVec2(projected.screenPosition), projected.screenRadius, enemyColor, 32, 2.0f);
	}
}
#endif
}

void SpawnRewardOrbFromEnemy(
	No::Registry& registry,
	No::Entity enemyEntity,
	CommentBoutCollision::Collider3DComponent& collider3D,
	No::CameraComponent& camera,
	const NoEngine::WindowSize& windowSize,
	const No::Vector2& bossBarAnchor,
	NoEngine::TextureRef rewardOrbTexture
) {
	const float worldSize = (collider3D.shapeType == CommentBoutCollision::ShapeType3D::Box)
		? std::max({ collider3D.worldBoxSize.x, collider3D.worldBoxSize.y, collider3D.worldBoxSize.z })
		: collider3D.worldRadius * 2.0f;

	No::Vector2 start = CommentBoutCollision::CoordinateConverter::WorldToScreen(
		collider3D.worldPosition,
		camera.forGPU.viewProjection,
		windowSize
	);
	if (!CommentBoutCollision::CoordinateConverter::IsValidProjection(start)) {
		return;
	}

	const float projectedRadius = CommentBoutCollision::CoordinateConverter::WorldRadiusToScreen(
		collider3D.worldPosition,
		std::max(0.1f, worldSize * 0.5f),
		camera.forGPU.viewProjection,
		windowSize
	);
	const float spriteSize = std::max(24.0f, std::min(180.0f, projectedRadius * 2.2f));
	const int attackPower = std::max(1, std::min(30, static_cast<int>(spriteSize / 18.0f)));

	auto orb = registry.GenerateEntity();
	registry.AddComponent<CBEnemyRewardOrbTag>(orb);
	auto* t2d = registry.AddComponent<No::Transform2DComponent>(orb);
	t2d->translate = start;
	t2d->scale = { spriteSize, spriteSize };
	auto* sprite = registry.AddComponent<No::SpriteComponent>(orb);
	sprite->textureHandle = rewardOrbTexture;
	sprite->layer = 91;
	sprite->orderInLayer = 20;
	sprite->color = { 1.0f, 0.9f, 0.2f, 0.95f };

	auto* reward = registry.AddComponent<EnemyRewardOrbComponent>(orb);
	reward->start = start;
	reward->end = bossBarAnchor;
	reward->control = { (start.x + bossBarAnchor.x) * 0.5f, std::min(start.y, bossBarAnchor.y) - 120.0f };
	reward->duration = 0.9f;
	reward->elapsed = 0.0f;
	reward->attackPower = attackPower;

	auto* src = registry.GetComponent<EnemyRewardSourceComponent>(enemyEntity);
	if (src) {
		src->spawned = true;
	}
}

void EnemyVisualSystem::Update(No::Registry& registry, float deltaTime)
{
	static_cast<void>(deltaTime);
	static bool drawColliderDebug = true;
	static bool drawSpriteGateDebug = true;
	static bool drawCameraGateDebugFlag = true;

	No::CameraComponent* activeCamera = nullptr;
	No::TransformComponent* activeCameraTransform = nullptr;
	auto activeCameraView = registry.View<No::CameraComponent, No::TransformComponent, No::ActiveCameraTag>();
	auto activeCameraIt = activeCameraView.begin();
	if (activeCameraIt != activeCameraView.end()) {
		activeCamera = registry.GetComponent<No::CameraComponent>(*activeCameraIt);
		activeCameraTransform = registry.GetComponent<No::TransformComponent>(*activeCameraIt);
	}

	PlayerHitboxComponent* debugHitbox = nullptr;
	auto debugHitboxView = registry.View<CBPlayerHitboxTag, PlayerHitboxComponent>();
	auto debugHitboxIt = debugHitboxView.begin();
	if (debugHitboxIt != debugHitboxView.end()) {
		debugHitbox = registry.GetComponent<PlayerHitboxComponent>(*debugHitboxIt);
	}

	GameResourceComponent* gameResource = nullptr;
	auto resourceView = registry.View<CBGameResourceTag, GameResourceComponent>();
	for (auto e : resourceView) {
		gameResource = registry.GetComponent<GameResourceComponent>(e);
		if (gameResource) {
			break;
		}
	}

	No::Vector2 bossBarAnchor = { 640.0f, 64.0f };
	auto bossBarView = registry.View<CBBossHpBarTag, HpBarComponent>();
	for (auto e : bossBarView) {
		auto* bar = registry.GetComponent<HpBarComponent>(e);
		if (bar) {
			bossBarAnchor = bar->anchor;
			break;
		}
	}

	auto* mainWindow = NoEngine::GraphicsCore::gWindowManager.GetMainWindow();
	NoEngine::WindowSize windowSize{};
	if (mainWindow) {
		windowSize = mainWindow->GetWindowSize();
	}

#ifdef USE_IMGUI
	ImGui::Begin("Enemy Debug");
	ImGui::Checkbox("敵コライダー表示", &drawColliderDebug);
	ImGui::Checkbox("重なりオーバーレイ表示", &drawSpriteGateDebug);
	ImGui::Checkbox("ゲートワイヤー表示", &drawCameraGateDebugFlag);
	ImGui::Separator();
	if (debugHitbox) {
		ImGui::Text("[カメラゲート設定(表示のみ)]");
		ImGui::Text("ゲート判定: %s", debugHitbox->useCameraGateForPlayerHit ? "ON" : "OFF");
		ImGui::Text("Near: %.2f", debugHitbox->cameraGateNear);
		ImGui::Text("Depth: %.2f", debugHitbox->cameraGateDepth);
		ImGui::Text("Half Width: %.2f", debugHitbox->cameraGateHalfWidth);
		ImGui::Text("Half Height: %.2f", debugHitbox->cameraGateHalfHeight);
		ImGui::Separator();
	}
#endif

	if (drawCameraGateDebugFlag && activeCameraTransform && debugHitbox && debugHitbox->useCameraGateForPlayerHit) {
		DrawCameraGateDebug(*activeCameraTransform, *debugHitbox);
	}

	auto view = registry.View<CBRailEnemyTag, EnemyComponent, HealthComponent, CommentBoutCollision::Collider3DComponent>();
	for (auto entity : view) {
		auto* enemy = registry.GetComponent<EnemyComponent>(entity);
		auto* health = registry.GetComponent<HealthComponent>(entity);
		auto* collider3D = registry.GetComponent<CommentBoutCollision::Collider3DComponent>(entity);
		auto* rewardSource = registry.GetComponent<EnemyRewardSourceComponent>(entity);
		auto* flash = registry.GetComponent<DamageFlashComponent>(entity);
		if (!enemy || !health || !collider3D) {
			continue;
		}

		enemy->hp = health->hp;
		enemy->maxHp = health->maxHp;
		enemy->lastDamageTaken = health->lastDamageTaken;

		if (health->isDead || health->hp <= 0) {
			if (enemy->removeReason == EnemyRemoveReason::None) {
				enemy->removeReason = EnemyRemoveReason::Defeated;
			}
			const bool defeated = (enemy->removeReason == EnemyRemoveReason::Defeated);
			if (defeated && !registry.Has<CBBossTag>(entity) && activeCamera && gameResource && mainWindow && rewardSource && !rewardSource->spawned) {
				SpawnRewardOrbFromEnemy(
					registry,
					entity,
					*collider3D,
					*activeCamera,
					windowSize,
					bossBarAnchor,
					GetGameTextureOrWhite(*gameResource, CommentBoutResourceKey::kRewardOrbSprite)
				);
			}
			registry.DestroyEntity(entity);
			continue;
		}

		enemy->wasCollidingWithAttack = (flash && flash->timer > 0.0f);

		bool gatePassForDebug = true;
		if (activeCameraTransform && debugHitbox && debugHitbox->useCameraGateForPlayerHit) {
			float gateCamX = 0.0f;
			float gateCamY = 0.0f;
			float gateCamZ = 0.0f;
			gatePassForDebug = CheckEnemyInCameraGate(collider3D->worldPosition, *activeCameraTransform, *debugHitbox, gateCamX, gateCamY, gateCamZ);
		}

		HitDebugState debugState = HitDebugState::None;
		if (collider3D->isColliding || enemy->wasCollidingWithAttack) {
			debugState = gatePassForDebug ? HitDebugState::InRangeHit : HitDebugState::OutOfRangeHit;
		}

		auto* enemyProjected = registry.GetComponent<CommentBoutCollision::ProjectedColliderComponent>(entity);
		const No::Transform2DComponent* spritePlayerTransform = nullptr;
		bool spriteOverlap = false;
		bool spriteGatePass = false;
		auto hitboxView = registry.View<CBPlayerHitboxTag, PlayerHitboxComponent>();
		for (auto hitboxEntity : hitboxView) {
			auto* hitbox = registry.GetComponent<PlayerHitboxComponent>(hitboxEntity);
			if (!hitbox || !hitbox->useCameraGateForPlayerHit || !enemyProjected || !activeCamera || !activeCameraTransform) {
				continue;
			}
			auto* playerTransform2D = registry.GetComponent<No::Transform2DComponent>(hitbox->playerEntity);
			if (!playerTransform2D) {
				continue;
			}
			spritePlayerTransform = playerTransform2D;
			spriteOverlap = CheckProjectedVsPlayerSprite(*enemyProjected, *playerTransform2D);
			float gateCamX = 0.0f;
			float gateCamY = 0.0f;
			float gateCamZ = 0.0f;
			spriteGatePass = CheckEnemyInCameraGate(collider3D->worldPosition, *activeCameraTransform, *hitbox, gateCamX, gateCamY, gateCamZ);
			break;
		}

#ifdef USE_IMGUI
		if (drawSpriteGateDebug && enemyProjected && spritePlayerTransform && debugHitbox && debugHitbox->useCameraGateForPlayerHit) {
			HitDebugState overlayState = HitDebugState::None;
			if (spriteOverlap) {
				overlayState = spriteGatePass ? HitDebugState::InRangeHit : HitDebugState::OutOfRangeHit;
			}
			DrawSpriteGateDebugOverlay(*enemyProjected, *spritePlayerTransform, overlayState);
		}

		const float flashTime = flash ? flash->timer : 0.0f;
		ImGui::Text("Enemy %llu hp=%d/%d flash=%.2f playerHit=%s",
			static_cast<unsigned long long>(entity),
			enemy->hp,
			enemy->maxHp,
			flashTime,
			enemy->wasCollidingWithPlayer ? "true" : "false");
		ImGui::Text("Collider pos(%.2f, %.2f, %.2f)", collider3D->worldPosition.x, collider3D->worldPosition.y, collider3D->worldPosition.z);
		if (collider3D->shapeType == CommentBoutCollision::ShapeType3D::Box) {
			ImGui::Text("Collider box(%.2f, %.2f, %.2f)", collider3D->worldBoxSize.x, collider3D->worldBoxSize.y, collider3D->worldBoxSize.z);
		} else {
			ImGui::Text("Collider radius=%.2f", collider3D->worldRadius);
		}
		ImGui::Separator();
#endif

		if (drawColliderDebug) {
			DrawColliderDebug(*collider3D, ToColliderDebugColor(debugState));
		}
	}

#ifdef USE_IMGUI
	ImGui::End();
#endif
}
