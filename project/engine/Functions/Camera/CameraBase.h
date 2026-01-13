#pragma once
#include "engine/Math/Types/Transform.h"
namespace NoEngine {
class CameraBase {
public:
	/// <summary>
	/// この関数で全ての行列が再生成されます。
	/// </summary>
	void Update();

	void SetTransform(const Transform& transform);

	[[nodiscard]] const Transform& GetTransform() const noexcept { return transform_; }
	[[nodiscard]] const Matrix4x4& GetViewMatrix() const noexcept { return viewMatrix_; }
	[[nodiscard]] const Matrix4x4& GetProjMatrix() const noexcept { return projectionMatrix_; }
	[[nodiscard]] const Matrix4x4& GetViewProjMatrix() const noexcept { return viewProjectionMatrix_; }
protected:
	Transform transform_;

	Matrix4x4 worldMatrix_;

	Matrix4x4 viewMatrix_;
	Matrix4x4 projectionMatrix_;
	Matrix4x4 viewProjectionMatrix_;
	
};
}

