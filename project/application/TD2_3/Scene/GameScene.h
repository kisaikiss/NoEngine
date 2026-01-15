#pragma once
#include "engine/NoEngine.h"

class GameScene : public No::IScene
{
public:
	void Setup() override;
private:
	std::unique_ptr<NoEngine::Camera> camera_;
	NoEngine::Transform cameraTransform_{};
	void NotSystemUpdate() override;

	void InitPlayer(No::Registry& registry);
	void InitEnemy(No::Registry& registry);
};