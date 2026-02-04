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
	void InitBackGround(No::Registry& registry);
	void InitBatGirl(No::Registry& registry);
	void InitPlayerGirl(No::Registry& registry);
	void InitChef(No::Registry& registry);
	void InitHumanParent(No::Registry& registry);
	void InitPlayerStatus(No::Registry& registry);
	void InitLights(No::Registry& registry);
	void InitHpGaugeSprite(No::Registry& registry);
	void InitLevelGaugeSprite(No::Registry& registry);
	void InitScore(No::Registry& registry);
	void InitBat(No::Registry& registry);
	void InitPhase(No::Registry& registry);


	void InitChooseSprite(No::Registry& registry);
	No::Entity CreateSprite(No::Registry& registry, const std::string& fileName, const std::string& configName);
	void DestroyGameObject();
	NoEngine::Vector3 GenerateRandomPointInCircle(float minRadius, float maxRadius);

	void SoundLoad();
	void ModelLoad();
};

