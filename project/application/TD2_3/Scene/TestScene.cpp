#include "TestScene.h"
#include "application/TD2_3/System/TestSystem.h"

void TestScene::Setup() {
	AddSystem(std::make_unique<TestSystem>());

	No::Registry& registry = *GetRegistry();
	No::Entity entity = registry.GenerateEntity();
	registry.AddComponent<No::TransformComponent>(entity);
	auto* model = registry.AddComponent<No::MeshComponent>(entity);
	model->mesh = NoEngine::ModelLoader::LoadModel("enemy", "resources/engine/Model/enemy.obj");

	auto m = registry.AddComponent<No::MaterialComponent>(entity);
	m->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/engine/Model/enemy.png");
	m->pso = &NoEngine::Render::GetPSO(L"Renderer : Default PSO");

	camera_ = std::make_unique<NoEngine::Camera>();
	cameraTransform_.translate.z = -5.f;
	camera_->SetTransform(cameraTransform_);
	SetCamera(camera_.get());
}

void TestScene::NotSystemUpdate() {
#ifdef USE_IMGUI
	ImGui::Begin("camera");
	ImGui::DragFloat3("pos", &cameraTransform_.translate.x, 0.1f);
	ImGui::End();
	camera_->SetTransform(cameraTransform_);
#endif // USE_IMGUI

	camera_->Update();
}
