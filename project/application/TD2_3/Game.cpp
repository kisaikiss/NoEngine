#include "Game.h"
#include "Scene/TestScene.h"

namespace {
float angle = 0.f;
}

void Game::Startup(void) {
	RegisterScene("TestScene", []() {
		return std::make_unique<TestScene>();
		});
	ChangeScene("TestScene");
}

void Game::Cleanup(void) {
	ShutdownSceneManager();
}

void Game::Update(float deltaT) {
	UpdateScene(deltaT);
#ifdef USE_IMGUI

	bool isPress = No::Keyboard::IsPress('A');
	ImGui::Begin("InputKeys");
	ImGui::Checkbox("isPressA", &isPress);
	ImGui::End();
#endif // USE_IMGUI

}

