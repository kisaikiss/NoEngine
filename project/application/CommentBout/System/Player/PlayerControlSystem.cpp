#include "stdafx.h"
#include "PlayerControlSystem.h"
#include "application/CommentBout/Component/Player/PlayerComponent.h"
#include "application/CommentBout/Component/Player/PlayerAttackComponent.h"
#include "application/CommentBout/Component/OutGame/PauseStateComponent.h"
#include "application/CommentBout/Component/LifetimeComponent.h"
#include "application/CommentBout/Component/GameResourceComponent.h"
#include "application/CommentBout/Component/AttackDamageComponent.h"
#include "application/CommentBout/Component/HealthComponent.h"
#include "application/CommentBout/Component/Player/InvincibleComponent.h"
#include "application/CommentBout/Utility/CBCollisionMask.h"
#include "application/CommentBout/Utility/InputHelper.h"
#include "application/CommentBout/Utility/CBGameAudio.h"
#include "application/CommentBout/Component/Player/PlayerAnimStateComponent.h"
#include "application/CommentBout/Data/PlayerConfig.h"
#include "application/CommentBout/GameTag.h"
#include "application/CommentBout/Collision/Component/Collider2DComponent.h"
#include "engine/Runtime/GraphicsCore.h"
#include <algorithm>
#include <cmath>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4702)
#endif

namespace {
No::Vector2 NormalizeOrZero(const No::Vector2& v) {
	const float lengthSq = v.x * v.x + v.y * v.y;
	if (lengthSq <= 0.000001f) {
		return { 0.0f, 0.0f };
	}
	const float invLen = 1.0f / std::sqrt(lengthSq);
	return { v.x * invLen, v.y * invLen };
}

float VectorLength(const No::Vector2& v) {
	return std::sqrt(v.x * v.x + v.y * v.y);
}
}

void PlayerControlSystem::Update(No::Registry& registry, float deltaTime)
{
	// ポーズ中はプレイヤー操作・攻撃生成を停止
	bool isPaused = false;
	auto pauseView = registry.View<CBPauseStateTag, PauseStateComponent>();
	for (auto pauseEntity : pauseView) {
		auto* pauseState = registry.GetComponent<PauseStateComponent>(pauseEntity);
		if (pauseState) {
			isPaused = pauseState->isPaused;
			break;
		}
	}
	if (isPaused) {
		return;
	}

	// 攻撃エフェクト生成に使う共有リソース
	GameResourceComponent* gameResource = nullptr;
	auto resourceView = registry.View<CBGameResourceTag, GameResourceComponent>();
	for (auto entity : resourceView) {
		gameResource = registry.GetComponent<GameResourceComponent>(entity);
		if (gameResource) {
			break;
		}
	}

	const PlayerConfig* playerConfig = nullptr;
	for (auto e : registry.View<CBPlayerConfigTag, PlayerConfig>()) {
		playerConfig = registry.GetComponent<PlayerConfig>(e);
		break;
	}

	auto playerView = registry.View<CBPlayerTag, PlayerComponent, PlayerAttackComponent, PlayerAnimStateComponent, HealthComponent, InvincibleComponent, No::Transform2DComponent>();
	for (auto entity : playerView) {
		auto* player = registry.GetComponent<PlayerComponent>(entity);
		auto* attack = registry.GetComponent<PlayerAttackComponent>(entity);
		auto* animState = registry.GetComponent<PlayerAnimStateComponent>(entity);
		auto* health = registry.GetComponent<HealthComponent>(entity);
		auto* invincible = registry.GetComponent<InvincibleComponent>(entity);
		auto* transform2D = registry.GetComponent<No::Transform2DComponent>(entity);
		if (!player || !attack || !animState || !health || !invincible || !transform2D) {
			continue;
		}

		if (invincible->duration <= 0.0f) {
			invincible->duration = std::max(0.01f, player->invincibleDurationDefault);
		}

		// HPが0以下でまだ死亡処理がされていない場合、死亡フラグを立てる
		// (シーン遷移は GameResultSystem が担当)
		if (health->hp <= 0 && !health->deathHandled) {
			health->isDead = true;
			health->deathHandled = true;
			continue;
		}

		if (health->isDead) {
			continue;
		}

		const No::Vector2 inputDir = NormalizeOrZero(InputHelper::GetMoveInput());

		const float maxSpeed = std::max(1.0f, (player->maxSpeed > 0.0f) ? player->maxSpeed : player->moveSpeed);
		const float acceleration = std::max(0.0f, player->acceleration);
		const float deceleration = std::max(0.0f, player->deceleration);

		if (inputDir.x != 0.0f || inputDir.y != 0.0f) {
			const No::Vector2 targetVelocity = inputDir * maxSpeed;
			No::Vector2 delta = targetVelocity - player->velocity;
			const float deltaLen = VectorLength(delta);
			const float maxStep = acceleration * std::max(0.0f, deltaTime);
			if (deltaLen <= maxStep || deltaLen <= 0.000001f) {
				player->velocity = targetVelocity;
			} else {
				delta = NormalizeOrZero(delta);
				player->velocity += delta * maxStep;
			}
		} else {
			const float speed = VectorLength(player->velocity);
			const float drop = deceleration * std::max(0.0f, deltaTime);
			const float nextSpeed = std::max(0.0f, speed - drop);
			if (nextSpeed <= 0.000001f || speed <= 0.000001f) {
				player->velocity = { 0.0f, 0.0f };
			} else {
				player->velocity = NormalizeOrZero(player->velocity) * nextSpeed;
			}
		}

		transform2D->translate += player->velocity * deltaTime;

		float windowWidth = 1280.0f;
		float windowHeight = 720.0f;
		auto* mainWindow = NoEngine::GraphicsCore::gWindowManager.GetMainWindow();
		if (mainWindow) {
			const auto& windowSize = mainWindow->GetWindowSize();
			windowWidth = static_cast<float>(windowSize.clientWidth);
			windowHeight = static_cast<float>(windowSize.clientHeight);
			const float halfWidth = transform2D->scale.x * 0.5f;
			const float halfHeight = transform2D->scale.y * 0.5f;

			const float clampedX = std::max(halfWidth, std::min(windowWidth - halfWidth, transform2D->translate.x));
			const float clampedY = std::max(halfHeight, std::min(windowHeight - halfHeight, transform2D->translate.y));
			if (clampedX != transform2D->translate.x) {
				player->velocity.x = 0.0f;
			}
			if (clampedY != transform2D->translate.y) {
				player->velocity.y = 0.0f;
			}
			transform2D->translate.x = clampedX;
			transform2D->translate.y = clampedY;
		}

#ifdef USE_IMGUI

		// デバッグ用無敵トグル（左右Shift同時押し）
		const bool shiftToggle =
			(No::Keyboard::IsTrigger(VK_LSHIFT) && No::Keyboard::IsPress(VK_RSHIFT)) ||
			(No::Keyboard::IsTrigger(VK_RSHIFT) && No::Keyboard::IsPress(VK_LSHIFT));
		if (shiftToggle) {
			player->debugInvincible = !player->debugInvincible;
		}
		if (player->debugInvincible) {
			// デバッグ無敵中は常に無敵時間を維持して、被弾で途切れないようにする。
			invincible->time = std::max(invincible->time, 0.2f);
		}

		if (player->debugInvincible) {
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			if (viewport) {
				ImGui::SetNextWindowPos(
					ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - 16.0f, viewport->WorkPos.y + 16.0f),
					ImGuiCond_Always,
					ImVec2(1.0f, 0.0f)
				);
			}
			ImGui::SetNextWindowBgAlpha(0.35f);
			ImGui::Begin("PlayerInvincibleOverlay", nullptr,
				ImGuiWindowFlags_NoDecoration |
				ImGuiWindowFlags_AlwaysAutoResize |
				ImGuiWindowFlags_NoSavedSettings |
				ImGuiWindowFlags_NoMove |
				ImGuiWindowFlags_NoFocusOnAppearing |
				ImGuiWindowFlags_NoNav);
			ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "自機無敵中");
			ImGui::End();
		}
#endif

		if (InputHelper::IsConfirmTrigger() && !animState->isAttacking && gameResource) {
			// 攻撃モーション開始 (タイマーは PlayerAnimSystem が PlayerConfig から設定)
			animState->isAttacking = true;
			CommentBout::GameAudio::PlaySEClip(CommentBoutResourceKey::kSEPlayerPunch);
			auto attackEntity = registry.GenerateEntity();
			auto* attackTransform = registry.AddComponent<No::Transform2DComponent>(attackEntity);
			attackTransform->translate = {
				transform2D->translate.x + attack->spawnOffset.x,
				transform2D->translate.y + attack->spawnOffset.y
			};
			attackTransform->scale = attack->attackSize;

			auto* attackSprite = registry.AddComponent<No::SpriteComponent>(attackEntity);
			attackSprite->layer = attack->attackLayer;
			attackSprite->orderInLayer = 1000;
			attackSprite->textureHandle = GetGameTextureOrWhite(*gameResource, CommentBoutResourceKey::kPlayerAttackSprite);
			attackSprite->color = { 1.0f, 1.0f, 0.0f, 1.0f };

			auto* collider2D = registry.AddComponent<CommentBoutCollision::Collider2DComponent>(attackEntity);
			collider2D->useTransformAsSize = true;
			collider2D->localOffset    = playerConfig ? playerConfig->attackCollider.localOffset2D    : No::Vector2{ 0.f, 0.f };
			collider2D->sizeMultiplier = playerConfig ? playerConfig->attackCollider.sizeMultiplier2D : No::Vector2{ 1.f, 1.f };
			collider2D->collisionLayer = CommentBout::CollisionLayer::CBPlayerAttack;
			collider2D->collisionMask = CommentBout::CollisionMask::CBPlayerAttack;

			auto* attackDamage = registry.AddComponent<AttackDamageComponent>(attackEntity);
			attackDamage->damage = std::max(1, attack->attackPower);

			auto* lifetime = registry.AddComponent<LifetimeComponent>(attackEntity);
			lifetime->remainingTime = attack->visibleTime;
			registry.AddComponent<CBAttackEffectTag>(attackEntity);
		}
	}
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif


