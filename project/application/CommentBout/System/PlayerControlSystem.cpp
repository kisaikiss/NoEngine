#include "stdafx.h"
#include "PlayerControlSystem.h"
#include "application/CommentBout/Component/PlayerComponent.h"
#include "application/CommentBout/Component/PlayerAttackComponent.h"
#include "application/CommentBout/Component/LifetimeComponent.h"
#include "application/CommentBout/Component/GameResourceComponent.h"
#include "application/CommentBout/Utility/CBCollisionMask.h"
#include "application/CommentBout/GameTag.h"
#include "application/TestApp/Component/Collider2DComponent.h"
#include "engine/Runtime/GraphicsCore.h"
#include <algorithm>

void PlayerControlSystem::Update(No::Registry& registry, float deltaTime)
{
	GameResourceComponent* gameResource = nullptr;
	auto resourceView = registry.View<CBGameResourceTag, GameResourceComponent>();
	for (auto entity : resourceView) {
		gameResource = registry.GetComponent<GameResourceComponent>(entity);
		if (gameResource) {
			break;
		}
	}

	auto playerView = registry.View<CBPlayerTag, PlayerComponent, PlayerAttackComponent, No::Transform2DComponent>();
	for (auto entity : playerView) {
		auto* player = registry.GetComponent<PlayerComponent>(entity);
		auto* attack = registry.GetComponent<PlayerAttackComponent>(entity);
		auto* transform2D = registry.GetComponent<No::Transform2DComponent>(entity);
		if (!player || !attack || !transform2D) {
			continue;
		}

		No::Vector2 input{ 0.0f, 0.0f };
		if (No::Keyboard::IsPress('W')) { input.y -= 1.0f; }
		if (No::Keyboard::IsPress('S')) { input.y += 1.0f; }
		if (No::Keyboard::IsPress('A')) { input.x -= 1.0f; }
		if (No::Keyboard::IsPress('D')) { input.x += 1.0f; }

		transform2D->translate.x += input.x * player->moveSpeed * deltaTime;
		transform2D->translate.y += input.y * player->moveSpeed * deltaTime;

		auto* mainWindow = NoEngine::GraphicsCore::gWindowManager.GetMainWindow();
		if (mainWindow) {
			const auto& windowSize = mainWindow->GetWindowSize();
			const float width = static_cast<float>(windowSize.clientWidth);
			const float height = static_cast<float>(windowSize.clientHeight);
			const float halfWidth = transform2D->scale.x * 0.5f;
			const float halfHeight = transform2D->scale.y * 0.5f;

			transform2D->translate.x = std::max(halfWidth, std::min(width - halfWidth, transform2D->translate.x));
			transform2D->translate.y = std::max(halfHeight, std::min(height - halfHeight, transform2D->translate.y));
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

			auto* lifetime = registry.AddComponent<LifetimeComponent>(attackEntity);
			lifetime->remainingTime = attack->visibleTime;
			registry.AddComponent<CBAttackEffectTag>(attackEntity);
		}
	}
}
