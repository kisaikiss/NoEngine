#pragma once
#include "engine/Math/Types/Transform.h"
namespace NoEngine {
class CameraBase {
public:
	struct CameraForGPU {
		Math::Matrix4x4 viewProjection;
		Math::Vector3 worldPosition;
		float fov;
	};

	/// <summary>
	/// この関数で全ての行列が再生成されます。
	/// </summary>
	void Update();

	void SetTransform(const Transform& transform);

	virtual void SetAspectRatio(float aspectRatio) { (void)aspectRatio; }

	[[nodiscard]] const Transform& GetTransform() const noexcept { return transform_; }
	[[nodiscard]] const Math::Matrix4x4& GetViewMatrix() const noexcept { return viewMatrix_; }
	[[nodiscard]] const Math::Matrix4x4& GetProjMatrix() const noexcept { return projectionMatrix_; }
	[[nodiscard]] const Math::Matrix4x4& GetViewProjMatrix() const noexcept { return viewProjectionMatrix_; }
	[[nodiscard]] const CameraForGPU& GetCameraForGPU() const noexcept { return forGpu_; }
protected:
	Transform transform_;

	Math::Matrix4x4 worldMatrix_;

	Math::Matrix4x4 viewMatrix_;
	Math::Matrix4x4 projectionMatrix_;
	Math::Matrix4x4 viewProjectionMatrix_;
	CameraForGPU forGpu_;
};
}

