#include "Game.h"
#include "Scene/TestScene.h"
#include "Scene/GameScene.h"

#include "application/TD2_3/Render/BackGroundEffect.h"
#include "application/TD2_3/Render/BallTrailPass.h"


void Game::Startup(void) {
	RegisterScene("TestScene", []() {
		return std::make_unique<TestScene>();
		});
	RegisterScene("GameScene", []() {
		return std::make_unique<GameScene>();
		});
	ChangeScene("GameScene");

	// カスタム RenderPass の追加例

	AddRenderPass(std::make_unique<BackGroundEffectPass>());
	AddRenderPass(std::make_unique<BallTrailPass>());

}

void Game::Cleanup(void) {
	ShutdownSceneManager();
}

void Game::Update(float deltaT) {
	if (auto event = GetRegistry().PollEvent<No::SceneChangeEvent>()) {
		ChangeScene(event->nextScene);
	}

	UpdateScene(deltaT);
}

