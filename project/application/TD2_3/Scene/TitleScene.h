#pragma once
#include "engine/NoEngine.h"

class TitleScene : public No::IScene
{
public:
	void Setup() override;
private:
	std::unique_ptr<NoEngine::Camera> camera_;
	NoEngine::Transform cameraTransform_{};
	void NotSystemUpdate() override;

	void InitTitle(No::Registry& registry);
};