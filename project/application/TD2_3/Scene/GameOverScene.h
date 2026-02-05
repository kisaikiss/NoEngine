#pragma once
#include "engine/NoEngine.h"

class GameOverScene : public No::IScene
{
public:
	void Setup() override;
private:
	std::unique_ptr<NoEngine::Camera> camera_;
	NoEngine::Transform cameraTransform_{};
	void NotSystemUpdate() override;
	bool isChangeScene_ = false;

private:
	void InitPlayerScore();
	void InitRankingSprite();

	void InitPlayerGirl();
	void InitChef();
	void InitBatGirl();
};