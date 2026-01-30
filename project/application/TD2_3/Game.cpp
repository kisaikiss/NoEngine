#include "Game.h"
#include "Scene/TestScene.h"
#include "Scene/GameScene.h"

#include "application/TD2_3/Render/BackGroundEffect.h"

namespace {
float angle = 0.f;
}

void Game::Startup(void) {
	RegisterScene("TestScene", []() {
		return std::make_unique<TestScene>();
		});
	RegisterScene("GameScene", []() {
		return std::make_unique<GameScene>();
		});
	ChangeScene("TestScene");

	// カスタム RenderPass の追加例
	AddRenderPass(std::make_unique<BackGroundEffectPass>());
}

void Game::Cleanup(void) {
	ShutdownSceneManager();
}

void Game::Update(float deltaT) {
	UpdateScene(deltaT);
}

