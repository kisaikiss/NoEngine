#include "CameraBase.h"

namespace NoEngine {
void CameraBase::Update() {
	worldMatrix_.MakeAffine(transform_.scale, transform_.rotation, transform_.translate);
	viewMatrix_ = worldMatrix_;
	viewMatrix_.Inverse();
	viewProjectionMatrix_ = viewMatrix_ * projectionMatrix_;
}

void CameraBase::SetTransform(const Transform& transform) {
	transform_ = transform;
}


}