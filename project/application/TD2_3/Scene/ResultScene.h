#pragma once
#include "engine/NoEngine.h"

class ResultScene : public No::IScene
{
public:
	void Setup() override;
private:
	void NotSystemUpdate() override;
	bool isChangeScene_ = false;

private:
	void InitPlayerScore();
	void InitRankingSprite();

	void InitPlayerGirl();
	void InitChef();
};