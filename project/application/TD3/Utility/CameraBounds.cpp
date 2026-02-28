#include "CameraBounds.h"
#include "engine/Functions/Camera/CameraBase.h"
#include <cmath>

// ============================================================
//  GetVisibleBounds
//  指定されたZ位置でのカメラ可視範囲を計算
// ============================================================

void CameraBounds::GetVisibleBounds(
	const NoEngine::CameraBase* camera,
	float zPosition,
	float& outLeft,
	float& outRight,
	float& outBottom,
	float& outTop
) {
	if (!camera) {
		outLeft = outRight = outBottom = outTop = 0.0f;
		return;
	}

	// カメラの位置とProjection行列から計算
	const auto& cameraTransform = camera->GetTransform();
	const auto& projMatrix = camera->GetProjMatrix();

	// カメラからオブジェクトまでのZ距離を計算
	float zDistance = std::abs(zPosition - cameraTransform.translate.z);

	// Projection行列からFOVとアスペクト比を逆算
	// projMatrix.m[1][1] = 1 / tan(fovY / 2)
	// projMatrix.m[0][0] = projMatrix.m[1][1] / aspectRatio
	
	float tanHalfFovY = 1.0f / projMatrix.m[1][1];
	float aspectRatio = projMatrix.m[1][1] / projMatrix.m[0][0];

	// Z距離から可視範囲を計算
	float visibleHeight = 2.0f * zDistance * tanHalfFovY;
	float visibleWidth = visibleHeight * aspectRatio;

	// カメラ中心からの範囲
	float halfWidth = visibleWidth * 0.5f;
	float halfHeight = visibleHeight * 0.5f;

	// カメラのワールド座標を中心に範囲を設定
	outLeft = cameraTransform.translate.x - halfWidth;
	outRight = cameraTransform.translate.x + halfWidth;
	outBottom = cameraTransform.translate.y - halfHeight;
	outTop = cameraTransform.translate.y + halfHeight;
}

// ============================================================
//  IsInBounds
//  座標が可視範囲内かチェック
// ============================================================

bool CameraBounds::IsInBounds(
	const NoEngine::CameraBase* camera,
	const NoEngine::Math::Vector3& worldPosition,
	float offset
) {
	if (!camera) return false;

	float left, right, bottom, top;
	GetVisibleBounds(camera, worldPosition.z, left, right, bottom, top);

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
	const NoEngine::CameraBase* camera,
	const NoEngine::Math::Vector3& worldPosition,
	float offset,
	bool& outLooped
) {
	outLooped = false;

	if (!camera) return worldPosition;

	float left, right, bottom, top;
	GetVisibleBounds(camera, worldPosition.z, left, right, bottom, top);

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
