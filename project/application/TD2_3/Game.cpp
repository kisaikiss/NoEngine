#include "Game.h"
#include "Scene/GameScene.h"
#include "Scene/TitleScene.h"
#include "Scene/GameOverScene.h"
#include "Scene/ResultScene.h"

#include "application/TD2_3/Render/BackGroundEffect.h"
#include "application/TD2_3/Render/BallTrailPass.h"


void Game::Startup(void)
{
	RegisterScene("TitleScene", []()
		{
			return std::make_unique<TitleScene>();
		});
	RegisterScene("GameScene", []()
		{
			return std::make_unique<GameScene>();
		});
	RegisterScene("GameOverScene", []()
		{
			return std::make_unique<GameOverScene>();
		});
	RegisterScene("ResultScene", []()
		{
			return std::make_unique<ResultScene>();
		});
	ChangeScene("TitleScene");

	// カスタム RenderPass の追加例

}

void Game::Cleanup(void)
{
	ShutdownSceneManager();
}

//void Game::Update(float deltaT)
//{
//	if (auto event = GetRegistry().PollEvent<No::SceneChangeEvent>())
//	{
//		ChangeScene(event->nextScene, false);
//	}
//
//	//UpdateScene(deltaT);
//}

