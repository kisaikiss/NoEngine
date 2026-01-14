#include "Game.h"
#include "engine/Functions/ECS/Component/MeshComponent.h"
#include "engine/Functions/ECS/Component/TransformComponent.h"
#include "engine/Functions/ECS/Component/MaterialComponent.h"
#include "engine/Assets/ModelLoader.h"
#include "engine/Functions/Renderer/RenderSystem.h"
#include "engine/Functions/Input/Keyboard.h"

#include <externals/imgui/imgui.h>

namespace {
float angle = 0.f;
}

void Game::Startup(void) {
	NoEngine::ECS::Registry& registry = GetRegistry();
	entity_ = registry.GenerateEntity();
	registry.AddComponent<NoEngine::Component::TransformComponent>(entity_);
	auto* model = registry.AddComponent<NoEngine::Component::MeshComponent>(entity_);
	model->mesh = NoEngine::ModelLoader::LoadModel("enemy", "resources/engine/Model/enemy.obj");

	auto m = registry.AddComponent<NoEngine::Component::MaterialComponent>(entity_);
	m->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/engine/Model/enemy.png");
	m->pso = &NoEngine::Render::GetPSO(L"Renderer : Default PSO");
}

void Game::Update(float deltaT) {
	(void)deltaT;
#ifdef USE_IMGUI
	NoEngine::ECS::Registry& registry = GetRegistry();
	auto* a = registry.GetComponent<NoEngine::Component::TransformComponent>(entity_);
	auto* b = registry.GetComponent<NoEngine::Component::MaterialComponent>(entity_);
	angle += 0.1f * deltaT;
	a->rotation.FromAxisAngle(NoEngine::Vector3(0.f, 1.f, 0.f), angle);


	ImGui::Begin("model");
	ImGui::DragFloat3("translate", &a->translate.x, 0.05f);
	ImGui::DragFloat3("scale", &a->scale.x, 0.05f);
	ImGui::DragFloat4("rotate", &a->rotation.x, 0.04f);
	ImGui::DragFloat4("uv", &b->uv.x, 0.01f);
	ImGui::End();

	bool isPress = NoEngine::Input::Keyboard::IsPress('A');
	ImGui::Begin("InputKeys");
	ImGui::Checkbox("isPressA", &isPress);
	ImGui::End();
#endif // USE_IMGUI

}
