#include "SampleScene.h"

void SampleScene::Setup() {
	camera_ = std::make_unique<NoEngine::Camera>();
	cameraTransform_.translate.z = -5.f;
	camera_->SetTransform(cameraTransform_);
	SetCamera(camera_.get());
}

void SampleScene::NotSystemUpdate() {
	camera_->Update();
}
