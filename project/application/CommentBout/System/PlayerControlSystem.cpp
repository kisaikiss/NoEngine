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
#include "engine/Functions/ECS/Component/CameraComponent.h"
#include "engine/Functions/Renderer/Primitive.h"
#include "engine/Runtime/GraphicsCore.h"
#include <algorithm>
#include <cmath>

namespace No {
using ::NoEngine::Primitive;
}

namespace {
bool ScreenToWorldOnPlane(const No::Matrix4x4& viewProjection, float screenX, float screenY, float viewportWidth, float viewportHeight, float planeZ, No::Vector3& outWorld) {
	if (viewportWidth <= 0.0f || viewportHeight <= 0.0f) {
		return false;
	}

	const float ndcX = (screenX / viewportWidth) * 2.0f - 1.0f;
	const float ndcY = 1.0f - (screenY / viewportHeight) * 2.0f;

	No::Matrix4x4 inv = viewProjection;
	inv.Inverse();

	const No::Vector3 nearPoint = inv.Transform({ ndcX, ndcY, 0.0f });
	const No::Vector3 farPoint = inv.Transform({ ndcX, ndcY, 1.0f });
	No::Vector3 dir = farPoint - nearPoint;
	if (std::abs(dir.z) <= 0.000001f) {
		return false;
	}

	const float t = (planeZ - nearPoint.z) / dir.z;
	outWorld = nearPoint + dir * t;
	return true;
}
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

				bool fitted = false;
				if (hitbox->fitToSprite) {
					No::CameraComponent* camera = nullptr;
					auto activeCameraView = registry.View<No::CameraComponent, No::ActiveCameraTag>();
					auto cameraIt = activeCameraView.begin();
					if (cameraIt != activeCameraView.end()) {
						camera = registry.GetComponent<No::CameraComponent>(*cameraIt);
					}
					if (camera) {
						const float halfW = transform2D->scale.x * 0.5f;
						const float halfH = transform2D->scale.y * 0.5f;
						const float left = transform2D->translate.x - halfW;
						const float right = transform2D->translate.x + halfW;
						const float top = transform2D->translate.y - halfH;
						const float bottom = transform2D->translate.y + halfH;

						No::Vector3 p0{}, p1{}, p2{}, p3{};
						const bool ok0 = ScreenToWorldOnPlane(camera->forGPU.viewProjection, left, top, windowWidth, windowHeight, hitbox->spritePlaneZ, p0);
						const bool ok1 = ScreenToWorldOnPlane(camera->forGPU.viewProjection, right, top, windowWidth, windowHeight, hitbox->spritePlaneZ, p1);
						const bool ok2 = ScreenToWorldOnPlane(camera->forGPU.viewProjection, right, bottom, windowWidth, windowHeight, hitbox->spritePlaneZ, p2);
						const bool ok3 = ScreenToWorldOnPlane(camera->forGPU.viewProjection, left, bottom, windowWidth, windowHeight, hitbox->spritePlaneZ, p3);

						if (ok0 && ok1 && ok2 && ok3) {
							const float minX = std::min(std::min(p0.x, p1.x), std::min(p2.x, p3.x));
							const float maxX = std::max(std::max(p0.x, p1.x), std::max(p2.x, p3.x));
							const float minY = std::min(std::min(p0.y, p1.y), std::min(p2.y, p3.y));
							const float maxY = std::max(std::max(p0.y, p1.y), std::max(p2.y, p3.y));

							hitboxTransform->translate = {
								((minX + maxX) * 0.5f) + hitbox->worldOffset.x,
								((minY + maxY) * 0.5f) + hitbox->worldOffset.y,
								hitbox->spritePlaneZ + hitbox->worldOffset.z
							};
							hitboxTransform->scale = {
								std::max(0.01f, (maxX - minX) * hitbox->sizeMultiplier.x),
								std::max(0.01f, (maxY - minY) * hitbox->sizeMultiplier.y),
								std::max(0.01f, hitbox->worldSize.z * hitbox->sizeMultiplier.z)
							};
							fitted = true;
						}
					}
				}

				if (!fitted) {
					const float worldX = (transform2D->translate.x - (windowWidth * 0.5f)) / 120.0f;
					const float worldY = ((windowHeight * 0.5f) - transform2D->translate.y) / 120.0f;
					hitboxTransform->translate = {
						worldX + hitbox->worldOffset.x,
						1.0f + worldY + hitbox->worldOffset.y,
						hitbox->spritePlaneZ + hitbox->worldOffset.z
					};
					hitboxTransform->scale = {
						hitbox->worldSize.x * hitbox->sizeMultiplier.x,
						hitbox->worldSize.y * hitbox->sizeMultiplier.y,
						hitbox->worldSize.z * hitbox->sizeMultiplier.z
					};
				}

				if (hitbox->drawDebug) {
					const NoEngine::Math::Color color = hitboxCollider->isColliding
						? NoEngine::Math::Color(1.0f, 0.2f, 0.2f, 1.0f)
						: NoEngine::Math::Color(0.9f, 0.9f, 1.0f, 1.0f);
					No::Primitive::DrawCube(hitboxTransform->translate, hitboxTransform->scale, color);
				}
			}
		}
	}
}
