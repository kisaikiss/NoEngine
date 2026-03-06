#include "CameraBounds.h"
#include "engine/NoEngine.h"
#include <cmath>

// ============================================================
//  GetVisibleBounds
//  指定されたZ位置でのカメラ可視範囲を計算
// ============================================================

void CameraBounds::GetVisibleBounds(
	NoEngine::ECS::Registry& registry,
	float zPosition,
	float& outLeft,
	float& outRight,
	float& outBottom,
	float& outTop
) {
	// カメラエンティティを取得
	auto cameraView = registry.View<No::ActiveCameraTag, No::CameraComponent, No::TransformComponent>();
	auto it = cameraView.begin();
	if (it == cameraView.end()) {
		outLeft = outRight = outBottom = outTop = 0.0f;
		return;
	}

	auto* camera = registry.GetComponent<No::CameraComponent>(*it);
	auto* cameraTransform = registry.GetComponent<No::TransformComponent>(*it);
	if (!camera || !cameraTransform) {
		outLeft = outRight = outBottom = outTop = 0.0f;
		return;
	}

	// カメラからオブジェクトまでのZ距離を計算
	float zDistance = std::abs(zPosition - cameraTransform->translate.z);

	// FOVとアスペクト比から計算
	float tanHalfFovY = std::tan(camera->fov * 0.5f);
	float aspectRatio = camera->aspect;

	// Z距離から可視範囲を計算
	float visibleHeight = 2.0f * zDistance * tanHalfFovY;
	float visibleWidth = visibleHeight * aspectRatio;

	// カメラ中心からの範囲
	float halfWidth = visibleWidth * 0.5f;
	float halfHeight = visibleHeight * 0.5f;

	// カメラのワールド座標を中心に範囲を設定
	outLeft = cameraTransform->translate.x - halfWidth;
	outRight = cameraTransform->translate.x + halfWidth;
	outBottom = cameraTransform->translate.y - halfHeight;
	outTop = cameraTransform->translate.y + halfHeight;
}

// ============================================================
//  IsInBounds
//  座標が可視範囲内かチェック
// ============================================================

bool CameraBounds::IsInBounds(
	NoEngine::ECS::Registry& registry,
	const NoEngine::Math::Vector3& worldPosition,
	float offset
) {
	float left, right, bottom, top;
	GetVisibleBounds(registry, worldPosition.z, left, right, bottom, top);

	// オフセット分範囲を広げる
	left -= offset;
	right += offset;
	bottom -= offset;
	top += offset;

	return (worldPosition.x >= left && worldPosition.x <= right &&
		worldPosition.y >= bottom && worldPosition.y <= top);
}

// ============================================================
//  LoopPosition
//  画面外に出た座標を反対側にループ
// ============================================================

NoEngine::Math::Vector3 CameraBounds::LoopPosition(
	NoEngine::ECS::Registry& registry,
	const NoEngine::Math::Vector3& worldPosition,
	float offset,
	bool& outLooped
) {
	outLooped = false;

	float left, right, bottom, top;
	GetVisibleBounds(registry, worldPosition.z, left, right, bottom, top);

	// オフセット分範囲を広げる
	left -= offset;
	right += offset;
	bottom -= offset;
	top += offset;

	NoEngine::Math::Vector3 newPosition = worldPosition;

	// X方向のループ
	if (worldPosition.x < left) {
		newPosition.x = right;
		outLooped = true;
	} else if (worldPosition.x > right) {
		newPosition.x = left;
		outLooped = true;
	}

	// Y方向のループ
	if (worldPosition.y < bottom) {
		newPosition.y = top;
		outLooped = true;
	} else if (worldPosition.y > top) {
		newPosition.y = bottom;
		outLooped = true;
	}

	return newPosition;
}
