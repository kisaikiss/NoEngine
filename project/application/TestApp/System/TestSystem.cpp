#include "TestSystem.h"
#include "engine/Runtime/GraphicsCore.h"
#include <algorithm>

void TestSystem::Update(No::Registry& registry, float deltaTime) {

	static_cast<void>(deltaTime);
	// 1. カメラ情報を取得
	No::CameraComponent* camera = nullptr;
	No::TransformComponent* cameraTransform = nullptr;
	auto cameraView = registry.View<No::CameraComponent, No::TransformComponent, No::ActiveCameraTag>();
	for (auto cameraEntity : cameraView) {
		camera = registry.GetComponent<No::CameraComponent>(cameraEntity);
		cameraTransform = registry.GetComponent<No::TransformComponent>(cameraEntity);
		if (camera && cameraTransform) {
			break;
		}
	}

	if (!camera || !cameraTransform) {
		return;
	}

	// 2. ウィンドウサイズを取得
	NoEngine::WindowSize windowSize = NoEngine::GraphicsCore::gWindowManager.GetMainWindow()->GetWindowSize();

	// 3. Ballのワールド座標を取得
	No::Vector3 ballWorldPos{};
	float ballRadius = 0.5f;
	bool ballFound = false;

	auto ballView = registry.View<No::TransformComponent, No::EditTag>();
	for (auto ballEntity : ballView) {
		auto* tag = registry.GetComponent<No::EditTag>(ballEntity);
		if (tag && tag->name == "Ball") {
			auto* ballTransform = registry.GetComponent<No::TransformComponent>(ballEntity);
			if (ballTransform) {
				ballWorldPos = ballTransform->GetWorldPosition();
				ballRadius = ballTransform->scale.x;
				ballFound = true;
				
				//// Ballを回転させる（デバッグ用）
				//ballTransform->translate.x = std::sin(angle_) * 2.0f;
				//ballTransform->translate.y = std::cos(angle_) * 1.0f;
				//
				// Primitiveで球を描画
				NoEngine::Primitive::DrawSphere(ballWorldPos, ballRadius, { 0.0f, 1.0f, 0.0f, 1.0f });
				break;
			}
		}
	}

	if (!ballFound) {
		return;
	}

	// 4. ワールド座標をスクリーン座標に変換
	No::Vector2 screenPos = WorldToScreen(ballWorldPos, camera->forGPU.viewProjection, windowSize);

	// 5. スクリーン上のBall半径を計算（透視投影による距離補正）
	No::Vector3 cameraPos = cameraTransform->GetWorldPosition();
	float distance = std::sqrt(
		(ballWorldPos.x - cameraPos.x) * (ballWorldPos.x - cameraPos.x) +
		(ballWorldPos.y - cameraPos.y) * (ballWorldPos.y - cameraPos.y) +
		(ballWorldPos.z - cameraPos.z) * (ballWorldPos.z - cameraPos.z)
	);

	// 焦点距離を計算
	float focalLength = static_cast<float>(windowSize.clientHeight) / (2.0f * std::tan(camera->fov / 2.0f));
	float screenRadius = (ballRadius * focalLength) / distance;

	// 6. 当たり判定用Spriteとの衝突判定
	auto spriteView = registry.View<No::Transform2DComponent, No::SpriteComponent, No::EditTag>();
	for (auto spriteEntity : spriteView) {
		auto* tag = registry.GetComponent<No::EditTag>(spriteEntity);
		if (tag && tag->name == "CollisionSprite") {
			auto* t2d = registry.GetComponent<No::Transform2DComponent>(spriteEntity);
			auto* sprite = registry.GetComponent<No::SpriteComponent>(spriteEntity);

			if (!t2d || !sprite) continue;

			// Spriteの中心座標と半分のサイズを計算
			No::Vector2 spriteCenter = t2d->translate;
			No::Vector2 spriteHalfSize = { t2d->scale.x * 0.5f, t2d->scale.y * 0.5f };

			// 円と矩形の当たり判定
			bool isHit = CheckCircleAABBCollision(screenPos, screenRadius, spriteCenter, spriteHalfSize);

			// 色を変更
			if (isHit) {
				sprite->color = { 1.0f, 0.0f, 0.0f, 1.0f }; // 赤
			} else {
				sprite->color = { 0.0f, 0.0f, 1.0f, 1.0f }; // 青
			}
		}
	}

#ifdef USE_IMGUI
	ImGui::Begin("Collision Debug");
	ImGui::Text("Ball World Pos: (%.2f, %.2f, %.2f)", ballWorldPos.x, ballWorldPos.y, ballWorldPos.z);
	ImGui::Text("Ball Screen Pos: (%.2f, %.2f)", screenPos.x, screenPos.y);
	ImGui::Text("Screen Radius: %.2f", screenRadius);
	ImGui::Text("Distance: %.2f", distance);
	ImGui::Text("Camera Pos: (%.2f, %.2f, %.2f)", cameraPos.x, cameraPos.y, cameraPos.z);
	ImGui::End();
#endif
}

No::Vector2 TestSystem::WorldToScreen(const No::Vector3& worldPos, const No::Matrix4x4& viewProjection, const NoEngine::WindowSize& windowSize) {
	// ワールド座標をクリップ座標に変換
	float clipX = worldPos.x * viewProjection.m[0][0] + worldPos.y * viewProjection.m[1][0] + worldPos.z * viewProjection.m[2][0] + viewProjection.m[3][0];
	float clipY = worldPos.x * viewProjection.m[0][1] + worldPos.y * viewProjection.m[1][1] + worldPos.z * viewProjection.m[2][1] + viewProjection.m[3][1];
	float clipZ = worldPos.x * viewProjection.m[0][2] + worldPos.y * viewProjection.m[1][2] + worldPos.z * viewProjection.m[2][2] + viewProjection.m[3][2];
	float clipW = worldPos.x * viewProjection.m[0][3] + worldPos.y * viewProjection.m[1][3] + worldPos.z * viewProjection.m[2][3] + viewProjection.m[3][3];

	// クリップ座標が0以下の場合は画面外（カメラの後ろ）
	if (clipW <= 0.0f) {
		return { -1000.0f, -1000.0f }; // 画面外を示す値
	}

	// NDC座標に変換（-1 ~ 1）
	float ndcX = clipX / clipW;
	float ndcY = clipY / clipW;
	float ndcZ = clipZ / clipW;

	// NDCがクリッピング範囲外の場合は画面外
	if (ndcZ < 0.0f || ndcZ > 1.0f) {
		return { -1000.0f, -1000.0f };
	}

	// スクリーン座標に変換
	float screenX = (ndcX + 1.0f) * 0.5f * static_cast<float>(windowSize.clientWidth);
	float screenY = (1.0f - ndcY) * 0.5f * static_cast<float>(windowSize.clientHeight);

	return { screenX, screenY };
}

bool TestSystem::CheckCircleAABBCollision(const No::Vector2& circleCenter, float radius, const No::Vector2& rectCenter, const No::Vector2& rectHalfSize) {
	// AABBの範囲を計算
	float left = rectCenter.x - rectHalfSize.x;
	float right = rectCenter.x + rectHalfSize.x;
	float top = rectCenter.y - rectHalfSize.y;
	float bottom = rectCenter.y + rectHalfSize.y;

	// 円の中心からAABBの最近接点を求める
	float closestX = std::clamp(circleCenter.x, left, right);
	float closestY = std::clamp(circleCenter.y, top, bottom);

	// 円の中心と最近接点の距離を計算
	float dx = circleCenter.x - closestX;
	float dy = circleCenter.y - closestY;
	float distanceSquared = dx * dx + dy * dy;

	// 距離が半径以下なら衝突
	return distanceSquared <= (radius * radius);
}
