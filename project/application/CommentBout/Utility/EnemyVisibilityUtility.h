#pragma once
#include "application/CommentBout/Collision/Utility/CoordinateConverter.h"

namespace CommentBoutVisibility {

	inline bool IsWorldPositionVisibleOnScreen(
		const No::Vector3& worldPosition,
		No::CameraComponent& camera,
		const NoEngine::WindowSize& windowSize,
		float screenMargin = 24.0f
	) {
		const No::Vector2 screen = CommentBoutCollision::CoordinateConverter::WorldToScreen(
			worldPosition,
			camera.forGPU.viewProjection,
			windowSize
		);
		if (!CommentBoutCollision::CoordinateConverter::IsValidProjection(screen)) {
			return false;
		}

		const float minX = -screenMargin;
		const float minY = -screenMargin;
		const float maxX = static_cast<float>(windowSize.clientWidth) + screenMargin;
		const float maxY = static_cast<float>(windowSize.clientHeight) + screenMargin;
		return (screen.x >= minX && screen.x <= maxX && screen.y >= minY && screen.y <= maxY);
	}

	inline float ComputeForwardDistanceFromCamera(
		No::TransformComponent& targetTransform,
		No::TransformComponent& cameraTransform
	) {
		No::Matrix4x4 cameraWorld = cameraTransform.MakeAffineMatrix4x4();
		No::Vector3 forward = cameraWorld.TransformNormal(No::Vector3::FORWARD);
		if (forward.LengthSquared() <= 0.000001f) {
			forward = No::Vector3::FORWARD;
		} else {
			forward = forward.Normalize();
		}

		const No::Vector3 toTarget = targetTransform.GetWorldPosition() - cameraTransform.GetWorldPosition();
		return toTarget.Dot(forward);
	}

}
