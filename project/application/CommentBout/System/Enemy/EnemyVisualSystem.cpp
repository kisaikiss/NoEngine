#include "stdafx.h"
#include "EnemyVisualSystem.h"
#include "application/CommentBout/Component/Enemy/EnemyComponent.h"
#include "application/CommentBout/Component/HealthComponent.h"
#include "application/CommentBout/Component/Player/PlayerHitboxComponent.h"
#include "application/CommentBout/Component/Enemy/EnemyRewardSourceComponent.h"
#include "application/CommentBout/Component/Enemy/SpeechBubbleComponent.h"
#include "application/CommentBout/Data/SpeechBubbleConfig.h"
#include "application/CommentBout/Component/HpBarComponent.h"
#include "application/CommentBout/Component/DamageFlashComponent.h"
#include "application/CommentBout/Component/GameResourceComponent.h"
#include "application/CommentBout/Collision/Component/Collider3DComponent.h"
#include "application/CommentBout/Collision/Component/ProjectedColliderComponent.h"
#include "application/CommentBout/Collision/Utility/CollisionAlgorithms.h"
#include "application/CommentBout/Collision/Utility/CoordinateConverter.h"
#include "application/CommentBout/GameTag.h"
#include "application/CommentBout/Utility/CBSpriteLayer.h"
#include "application/CommentBout/Utility/CBGameAudio.h"
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

/// <summary>
/// 吹き出しスポーン関数。カメラからの距離で大中小を決定する。
/// </summary>
void SpawnSpeechBubbleFromEnemy(
	No::Registry& registry,
	No::Entity enemyEntity,
	CommentBoutCollision::Collider3DComponent& collider3D,
	No::CameraComponent& camera,
	No::TransformComponent& cameraTransform,
	const NoEngine::WindowSize& windowSize,
	const No::Vector2& bossBarAnchor,
	const SpeechBubbleConfig& sbConfig,
	const GameResourceComponent& gameResource
) {
	// 敵のスクリーン座標を計算
	No::Vector2 start = CommentBoutCollision::CoordinateConverter::WorldToScreen(
		collider3D.worldPosition,
		camera.forGPU.viewProjection,
		windowSize
	);
	if (!CommentBoutCollision::CoordinateConverter::IsValidProjection(start)) {
		return;
	}

	// カメラ距離でサイズを決定
	const No::Vector3 cameraPos = cameraTransform.GetWorldPosition();
	const float dist = (collider3D.worldPosition - cameraPos).Length();

	int sizeCategory = 2; // 2=Small（デフォルト）
	if (dist <= sbConfig.largeMaxDistance) {
		sizeCategory = 0; // Large
	} else if (dist <= sbConfig.mediumMaxDistance) {
		sizeCategory = 1; // Medium
	}

	// サイズ別設定・テクスチャ取得
	const SpeechBubbleSizeConfig* sizeCfg = &sbConfig.sizeSmall;
	const char* textureKey = CommentBoutResourceKey::kSpeechBubbleSmall;
	switch (sizeCategory) {
	case 0:
		sizeCfg    = &sbConfig.sizeLarge;
		textureKey = CommentBoutResourceKey::kSpeechBubbleLarge;
		break;
	case 1:
		sizeCfg    = &sbConfig.sizeMedium;
		textureKey = CommentBoutResourceKey::kSpeechBubbleMedium;
		break;
	default:
		break;
	}

	// 吹き出し SE
	switch (sizeCategory) {
	case 0: CommentBout::GameAudio::PlaySEClip(CommentBoutResourceKey::kSESpeechBubbleL); break;
	case 1: CommentBout::GameAudio::PlaySEClip(CommentBoutResourceKey::kSESpeechBubbleM); break;
	default: CommentBout::GameAudio::PlaySEClip(CommentBoutResourceKey::kSESpeechBubbleS); break;
	}

	// 吹き出しエンティティ生成
	auto bubble = registry.GenerateEntity();
	registry.AddComponent<CBSpeechBubbleTag>(bubble);

	auto* t2d = registry.AddComponent<No::Transform2DComponent>(bubble);
	t2d->translate = start;
	// 出現開始時は縮小スケール。SpeechBubbleToBossSystem の Appearing フェーズがバウンス拡大する。
	const float initScale = std::max(0.01f, sbConfig.initialScalePercent);
	t2d->scale     = sizeCfg->spriteSize * initScale;

	auto* sprite = registry.AddComponent<No::SpriteComponent>(bubble);
	sprite->textureHandle = GetGameTextureOrWhite(gameResource, textureKey);
	sprite->layer        = CommentBout::ToLayer(CommentBout::SpriteLayer::RewardOrb);
	sprite->orderInLayer = 20;
	sprite->color        = { 1.0f, 1.0f, 1.0f, 0.95f };

	auto* comp = registry.AddComponent<SpeechBubbleComponent>(bubble);
	comp->start        = start;
	comp->end          = bossBarAnchor;
	comp->control      = { (start.x + bossBarAnchor.x) * 0.5f, std::min(start.y, bossBarAnchor.y) - 120.0f };
	comp->duration     = sizeCfg->duration;
	comp->elapsed      = 0.0f;
	comp->attackPower  = sizeCfg->attackPower;
	comp->sizeCategory = sizeCategory;

	auto* src = registry.GetComponent<EnemyRewardSourceComponent>(enemyEntity);
	if (src) {
		src->spawned = true;
	}
}

void EnemyVisualSystem::Update(No::Registry& registry, float deltaTime)
{
	static_cast<void>(deltaTime);

	No::CameraComponent* activeCamera = nullptr;
	No::TransformComponent* activeCameraTransform = nullptr;
	auto activeCameraView = registry.View<No::CameraComponent, No::TransformComponent, No::ActiveCameraTag>();
	auto activeCameraIt = activeCameraView.begin();
	if (activeCameraIt != activeCameraView.end()) {
		activeCamera = registry.GetComponent<No::CameraComponent>(*activeCameraIt);
		activeCameraTransform = registry.GetComponent<No::TransformComponent>(*activeCameraIt);
	}

	GameResourceComponent* gameResource = nullptr;
	auto resourceView = registry.View<CBGameResourceTag, GameResourceComponent>();
	for (auto e : resourceView) {
		gameResource = registry.GetComponent<GameResourceComponent>(e);
		if (gameResource) {
			break;
		}
	}

	// 吹き出し設定取得
	SpeechBubbleConfig* sbConfig = nullptr;
	auto sbConfigView = registry.View<CBSpeechBubbleConfigTag, SpeechBubbleConfig>();
	for (auto e : sbConfigView) {
		sbConfig = registry.GetComponent<SpeechBubbleConfig>(e);
		if (sbConfig) break;
	}
	static SpeechBubbleConfig sSpeechBubbleConfigDefault; // ファイルがなければデフォルト
	if (!sbConfig) {
		sbConfig = &sSpeechBubbleConfigDefault;
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

	// 敵の状態更新・死亡時の吹き出しスポーン
	auto view = registry.View<CBRailEnemyTag, EnemyComponent, HealthComponent, CommentBoutCollision::Collider3DComponent>();
	for (auto entity : view) {
		auto* enemy      = registry.GetComponent<EnemyComponent>(entity);
		auto* health     = registry.GetComponent<HealthComponent>(entity);
		auto* collider3D = registry.GetComponent<CommentBoutCollision::Collider3DComponent>(entity);
		auto* rewardSource = registry.GetComponent<EnemyRewardSourceComponent>(entity);
		auto* flash      = registry.GetComponent<DamageFlashComponent>(entity);
		if (!enemy || !health || !collider3D) {
			continue;
		}

		enemy->hp             = health->hp;
		enemy->maxHp          = health->maxHp;
		enemy->lastDamageTaken = health->lastDamageTaken;

		if (health->isDead || health->hp <= 0) {
			if (enemy->removeReason == EnemyRemoveReason::None) {
				enemy->removeReason = EnemyRemoveReason::Defeated;
			}
			const bool defeated = (enemy->removeReason == EnemyRemoveReason::Defeated);
			if (defeated && !registry.Has<CBBossTag>(entity) && activeCamera && activeCameraTransform && gameResource && mainWindow && rewardSource && !rewardSource->spawned) {
				SpawnSpeechBubbleFromEnemy(
					registry,
					entity,
					*collider3D,
					*activeCamera,
					*activeCameraTransform,
					windowSize,
					bossBarAnchor,
					*sbConfig,
					*gameResource
				);
			}
			if (!health->deathHandled) {
				health->deathHandled = true;
				registry.DestroyEntity(entity);
			}
			continue;
		}

		enemy->wasCollidingWithAttack = (flash && flash->timer > 0.0f);
	}
}
