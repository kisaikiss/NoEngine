#include "stdafx.h"
#include "PlayerControlSystem.h"
#include "application/CommentBout/Component/PlayerComponent.h"
#include "application/CommentBout/Component/PlayerAttackComponent.h"
#include "application/CommentBout/Component/PauseStateComponent.h"
#include "application/CommentBout/Component/LifetimeComponent.h"
#include "application/CommentBout/Component/GameResourceComponent.h"
#include "application/CommentBout/Component/AttackDamageComponent.h"
#include "application/CommentBout/Component/PlayerHealthComponent.h"
#include "application/CommentBout/Component/PlayerHitboxComponent.h"
#include "application/CommentBout/Utility/CBCollisionMask.h"
#include "application/CommentBout/GameTag.h"
#include "application/TestApp/Component/Collider2DComponent.h"
#include "application/TestApp/Component/Collider3DComponent.h"
#include "engine/Functions/Renderer/Primitive.h"
#include "engine/Runtime/GraphicsCore.h"
#include <algorithm>

namespace No {
using ::NoEngine::Primitive;
}

void PlayerControlSystem::Update(No::Registry& registry, float deltaTime)
{
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

	GameResourceComponent* gameResource = nullptr;
	auto resourceView = registry.View<CBGameResourceTag, GameResourceComponent>();
	for (auto entity : resourceView) {
		gameResource = registry.GetComponent<GameResourceComponent>(entity);
		if (gameResource) {
			break;
		}
	}

	auto playerView = registry.View<CBPlayerTag, PlayerComponent, PlayerAttackComponent, PlayerHealthComponent, No::Transform2DComponent>();
	for (auto entity : playerView) {
		auto* player = registry.GetComponent<PlayerComponent>(entity);
		auto* attack = registry.GetComponent<PlayerAttackComponent>(entity);
		auto* health = registry.GetComponent<PlayerHealthComponent>(entity);
		auto* transform2D = registry.GetComponent<No::Transform2DComponent>(entity);
		if (!player || !attack || !health || !transform2D) {
			continue;
		}

		if (health->invincibleTime > 0.0f) {
			health->invincibleTime -= deltaTime;
			if (health->invincibleTime < 0.0f) {
				health->invincibleTime = 0.0f;
			}
		}

		if (health->hp <= 0 && !health->deathHandled) {
			health->isDead = true;
			health->deathHandled = true;
			No::SceneChangeEvent event;
			event.nextScene = "GameScene";
			registry.EmitEvent(event);
			continue;
		}

		if (health->isDead) {
			continue;
		}

		No::Vector2 input{ 0.0f, 0.0f };
		if (No::Keyboard::IsPress('W')) { input.y -= 1.0f; }
		if (No::Keyboard::IsPress('S')) { input.y += 1.0f; }
		if (No::Keyboard::IsPress('A')) { input.x -= 1.0f; }
		if (No::Keyboard::IsPress('D')) { input.x += 1.0f; }

		transform2D->translate.x += input.x * player->moveSpeed * deltaTime;
		transform2D->translate.y += input.y * player->moveSpeed * deltaTime;

		float windowWidth = 1280.0f;
		float windowHeight = 720.0f;
		auto* mainWindow = NoEngine::GraphicsCore::gWindowManager.GetMainWindow();
		if (mainWindow) {
			const auto& windowSize = mainWindow->GetWindowSize();
			windowWidth = static_cast<float>(windowSize.clientWidth);
			windowHeight = static_cast<float>(windowSize.clientHeight);
			const float halfWidth = transform2D->scale.x * 0.5f;
			const float halfHeight = transform2D->scale.y * 0.5f;

			transform2D->translate.x = std::max(halfWidth, std::min(windowWidth - halfWidth, transform2D->translate.x));
			transform2D->translate.y = std::max(halfHeight, std::min(windowHeight - halfHeight, transform2D->translate.y));
		}

		if (No::Keyboard::IsTrigger(VK_SPACE) && gameResource) {
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
			attackSprite->textureHandle = gameResource->whiteTexture;
			attackSprite->color = { 1.0f, 1.0f, 0.0f, 1.0f };

			auto* collider2D = registry.AddComponent<TestApp::Collider2DComponent>(attackEntity);
			collider2D->useTransformAsSize = true;
			collider2D->sizeMultiplier = { 1.0f, 1.0f };
			collider2D->collisionLayer = CommentBout::CollisionLayer::CBPlayerAttack;
			collider2D->collisionMask = CommentBout::CollisionMask::CBPlayerAttack;

			auto* attackDamage = registry.AddComponent<AttackDamageComponent>(attackEntity);
			attackDamage->damage = std::max(1, attack->attackPower);

			auto* lifetime = registry.AddComponent<LifetimeComponent>(attackEntity);
			lifetime->remainingTime = attack->visibleTime;
			registry.AddComponent<CBAttackEffectTag>(attackEntity);
		}

		auto* hitboxWindow = NoEngine::GraphicsCore::gWindowManager.GetMainWindow();
		if (hitboxWindow) {
			const auto& windowSize = hitboxWindow->GetWindowSize();
			windowWidth = static_cast<float>(windowSize.clientWidth);
			windowHeight = static_cast<float>(windowSize.clientHeight);

			auto hitboxView = registry.View<CBPlayerHitboxTag, PlayerHitboxComponent, No::TransformComponent, TestApp::Collider3DComponent>();
			for (auto hitboxEntity : hitboxView) {
				auto* hitbox = registry.GetComponent<PlayerHitboxComponent>(hitboxEntity);
				auto* hitboxTransform = registry.GetComponent<No::TransformComponent>(hitboxEntity);
				auto* hitboxCollider = registry.GetComponent<TestApp::Collider3DComponent>(hitboxEntity);
				if (!hitbox || !hitboxTransform || !hitboxCollider) {
					continue;
				}
				if (hitbox->playerEntity != entity) {
					continue;
				}

				const float worldX = (transform2D->translate.x - (windowWidth * 0.5f)) / 120.0f;
				const float worldY = ((windowHeight * 0.5f) - transform2D->translate.y) / 120.0f;
				hitboxTransform->translate = {
					worldX + hitbox->worldOffset.x,
					1.0f + worldY + hitbox->worldOffset.y,
					hitbox->worldOffset.z
				};
				hitboxTransform->scale = hitbox->worldSize;

				if (hitbox->drawDebug) {
					const NoEngine::Math::Color color = hitboxCollider->isColliding
						? NoEngine::Math::Color(1.0f, 0.2f, 0.2f, 1.0f)
						: NoEngine::Math::Color(0.2f, 0.9f, 1.0f, 1.0f);
					No::Primitive::DrawCube(hitboxTransform->translate, hitboxTransform->scale, color);
				}
			}
		}
	}
}
