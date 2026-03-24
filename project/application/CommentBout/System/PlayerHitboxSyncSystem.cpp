#include "stdafx.h"
#include "PlayerHitboxSyncSystem.h"

#include "application/CommentBout/Component/PlayerHitboxComponent.h"
#include "application/CommentBout/GameTag.h"
#include "application/CommentBout/Collision/Component/Collider3DComponent.h"
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

No::Vector3 NormalizeOrDefault(const No::Vector3& v, const No::Vector3& fallback) {
	if (v.LengthSquared() <= 0.000001f) {
		return fallback;
	}
	return v.Normalize();
}

void BuildObbFromQuad(PlayerHitboxComponent& hitbox, const No::Vector3& p0, const No::Vector3& p1, const No::Vector3& p2, const No::Vector3& p3, float halfDepth) {
	const No::Vector3 center = (p0 + p1 + p2 + p3) * 0.25f + hitbox.worldOffset;
	No::Vector3 axisX = NormalizeOrDefault(p1 - p0, No::Vector3::RIGHT);
	No::Vector3 edgeY = NormalizeOrDefault(p3 - p0, No::Vector3::UP);
	No::Vector3 axisZ = NormalizeOrDefault(axisX.Cross(edgeY), No::Vector3::FORWARD);
	No::Vector3 axisY = NormalizeOrDefault(axisZ.Cross(axisX), No::Vector3::UP);
	axisZ = NormalizeOrDefault(axisX.Cross(axisY), No::Vector3::FORWARD);

	hitbox.obbValid = true;
	hitbox.obbCenter = center;
	hitbox.obbAxisX = axisX;
	hitbox.obbAxisY = axisY;
	hitbox.obbAxisZ = axisZ;
	hitbox.obbHalfExtents = {
		std::max(0.005f, 0.5f * (p1 - p0).Length() * hitbox.sizeMultiplier.x),
		std::max(0.005f, 0.5f * (p3 - p0).Length() * hitbox.sizeMultiplier.y),
		std::max(0.005f, halfDepth)
	};
}

void BuildFallbackObb(PlayerHitboxComponent& hitbox, const No::TransformComponent& transform) {
	hitbox.obbValid = true;
	hitbox.obbCenter = transform.translate;
	hitbox.obbAxisX = No::Vector3::RIGHT;
	hitbox.obbAxisY = No::Vector3::UP;
	hitbox.obbAxisZ = No::Vector3::FORWARD;
	hitbox.obbHalfExtents = {
		std::max(0.005f, transform.scale.x * 0.5f),
		std::max(0.005f, transform.scale.y * 0.5f),
		std::max(0.005f, transform.scale.z * 0.5f)
	};
}

void DrawObbDebug(const PlayerHitboxComponent& hitbox, const NoEngine::Math::Color& color) {
	if (!hitbox.obbValid) {
		return;
	}

	const No::Vector3 ex = hitbox.obbAxisX * hitbox.obbHalfExtents.x;
	const No::Vector3 ey = hitbox.obbAxisY * hitbox.obbHalfExtents.y;
	const No::Vector3 ez = hitbox.obbAxisZ * hitbox.obbHalfExtents.z;
	const No::Vector3 c = hitbox.obbCenter;

	const No::Vector3 p000 = c - ex - ey - ez;
	const No::Vector3 p001 = c - ex - ey + ez;
	const No::Vector3 p010 = c - ex + ey - ez;
	const No::Vector3 p011 = c - ex + ey + ez;
	const No::Vector3 p100 = c + ex - ey - ez;
	const No::Vector3 p101 = c + ex - ey + ez;
	const No::Vector3 p110 = c + ex + ey - ez;
	const No::Vector3 p111 = c + ex + ey + ez;

	No::Primitive::DrawLine(p000, p001, color);
	No::Primitive::DrawLine(p001, p011, color);
	No::Primitive::DrawLine(p011, p010, color);
	No::Primitive::DrawLine(p010, p000, color);

	No::Primitive::DrawLine(p100, p101, color);
	No::Primitive::DrawLine(p101, p111, color);
	No::Primitive::DrawLine(p111, p110, color);
	No::Primitive::DrawLine(p110, p100, color);

	No::Primitive::DrawLine(p000, p100, color);
	No::Primitive::DrawLine(p001, p101, color);
	No::Primitive::DrawLine(p010, p110, color);
	No::Primitive::DrawLine(p011, p111, color);
}
}

void PlayerHitboxSyncSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);

	float windowWidth = 1280.0f;
	float windowHeight = 720.0f;
	auto* window = NoEngine::GraphicsCore::gWindowManager.GetMainWindow();
	if (window) {
		const auto& windowSize = window->GetWindowSize();
		windowWidth = static_cast<float>(windowSize.clientWidth);
		windowHeight = static_cast<float>(windowSize.clientHeight);
	}

	No::CameraComponent* activeCamera = nullptr;
	auto activeCameraView = registry.View<No::CameraComponent, No::ActiveCameraTag>();
	auto cameraIt = activeCameraView.begin();
	if (cameraIt != activeCameraView.end()) {
		activeCamera = registry.GetComponent<No::CameraComponent>(*cameraIt);
	}

	auto hitboxView = registry.View<CBPlayerHitboxTag, PlayerHitboxComponent, No::TransformComponent, CommentBoutCollision::Collider3DComponent>();
	for (auto hitboxEntity : hitboxView) {
		auto* hitbox = registry.GetComponent<PlayerHitboxComponent>(hitboxEntity);
		auto* hitboxTransform = registry.GetComponent<No::TransformComponent>(hitboxEntity);
		auto* hitboxCollider = registry.GetComponent<CommentBoutCollision::Collider3DComponent>(hitboxEntity);
		if (!hitbox || !hitboxTransform || !hitboxCollider) {
			continue;
		}

		auto* playerTransform2D = registry.GetComponent<No::Transform2DComponent>(hitbox->playerEntity);
		if (!playerTransform2D) {
			hitbox->obbValid = false;
			continue;
		}

		const float halfDepth = std::max(0.005f, 0.5f * hitbox->worldSize.z * hitbox->sizeMultiplier.z);
		bool fitted = false;
		if (hitbox->fitToSprite && activeCamera) {
			const float halfW = playerTransform2D->scale.x * 0.5f;
			const float halfH = playerTransform2D->scale.y * 0.5f;
			const float left = playerTransform2D->translate.x - halfW;
			const float right = playerTransform2D->translate.x + halfW;
			const float top = playerTransform2D->translate.y - halfH;
			const float bottom = playerTransform2D->translate.y + halfH;

			No::Vector3 p0{}, p1{}, p2{}, p3{};
			const bool ok0 = ScreenToWorldOnPlane(activeCamera->forGPU.viewProjection, left, top, windowWidth, windowHeight, hitbox->spritePlaneZ, p0);
			const bool ok1 = ScreenToWorldOnPlane(activeCamera->forGPU.viewProjection, right, top, windowWidth, windowHeight, hitbox->spritePlaneZ, p1);
			const bool ok2 = ScreenToWorldOnPlane(activeCamera->forGPU.viewProjection, right, bottom, windowWidth, windowHeight, hitbox->spritePlaneZ, p2);
			const bool ok3 = ScreenToWorldOnPlane(activeCamera->forGPU.viewProjection, left, bottom, windowWidth, windowHeight, hitbox->spritePlaneZ, p3);

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
				BuildObbFromQuad(*hitbox, p0, p1, p2, p3, halfDepth);
				fitted = true;
			}
		}

		if (!fitted) {
			const float worldX = (playerTransform2D->translate.x - (windowWidth * 0.5f)) / 120.0f;
			const float worldY = ((windowHeight * 0.5f) - playerTransform2D->translate.y) / 120.0f;
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
			BuildFallbackObb(*hitbox, *hitboxTransform);
		}

		if (hitbox->drawDebug) {
			const NoEngine::Math::Color color = hitboxCollider->isColliding
				? NoEngine::Math::Color(1.0f, 0.2f, 0.2f, 1.0f)
				: NoEngine::Math::Color(0.2f, 0.9f, 1.0f, 1.0f);
			DrawObbDebug(*hitbox, color);
		}
	}
}

