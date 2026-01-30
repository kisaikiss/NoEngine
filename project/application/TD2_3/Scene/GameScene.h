#pragma once
#include "engine/NoEngine.h"
#include"../Loader/EnemyResources.h"
class GameScene : public No::IScene
{
public:
	void Setup() override;
private:
	std::unique_ptr<NoEngine::Camera> camera_;
	NoEngine::Transform cameraTransform_{};

	EnemyResources enemyResources_;

	void NotSystemUpdate() override;

	void InitVaus(No::Registry& registry);
	void InitRing(No::Registry& registry);
	void InitBall(No::Registry& registry);
	void InitEnemy(No::Registry& registry);
	void InitBoss(No::Registry& registry);
	void InitBackGround(No::Registry& registry);
	void InitBatGirl(No::Registry& registry);
	void InitPlayerGirl(No::Registry& registry);
	void InitHpGaugeSprite(No::Registry& registry);
	void InitLevelGaugeSprite(No::Registry& registry);
	void CreateSprite(No::Registry& registry, NoEngine::Vector2 translate,
		NoEngine::Vector2 scale, const std::string& fileName, const std::string& configName);
	void DestroyGameObject();
	NoEngine::Vector3 GenerateRandomPointInCircle(float minRadius, float maxRadius);

	void SoundLoad();
};

