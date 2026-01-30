#include "TestScene.h"
#include "application/TD2_3/System/TestSystem.h"

void TestScene::Setup() {
	AddSystem(std::make_unique<TestSystem>());
	AddSystem(std::make_unique<No::AnimationSystem>());

	No::Registry& registry = *GetRegistry();
	No::Entity entity = registry.GenerateEntity();
	auto* t = registry.AddComponent<No::TransformComponent>(entity);
	auto* model = registry.AddComponent<No::MeshComponent>(entity);
	t->rotation.FromAxisAngle(NoEngine::Vector3(0.f, 1.f, 0.f), PI);
	auto* m = registry.AddComponent<No::MaterialComponent>(entity);
	auto* a = registry.AddComponent<No::AnimatorComponent>(entity);
	NoEngine::ModelLoader::LoadModel("magiclash", "resources/engine/Model/test/panda/panda.gltf");
	NoEngine::ModelLoader::GetModel("magiclash", model, a);
	m->materials = NoEngine::ModelLoader::GetMaterial("magiclash");
	a->currentAnimation = 1;
	
	m->psoName = L"Renderer : DefaultSkinned PSO";
	m->psoId = NoEngine::Render::GetPSOID(m->psoName);
	m->rootSigId = NoEngine::Render::GetRootSignatureID(m->psoName);

	auto* t2d = registry.AddComponent<No::Transform2DComponent>(entity);
	t2d->translate = { 100.f, 200.f };
	auto* sprite = registry.AddComponent<No::SpriteComponent>(entity);
	sprite->layer = 1;

	t2d->scale = { 100.f, 100.f };
	sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/engine/Model/enemy.png");



	auto light = registry.GenerateEntity();
	auto* dir = registry.AddComponent<No::DirectionalLightComponent>(light);
	dir->color = { 1.f,1.f,1.f,1.f };
	dir->direction = { 0.f,-1.f,0.f };
	dir->intensity = 1.f;

	auto* t2d2 = registry.AddComponent<No::Transform2DComponent>(light);
	t2d2->translate = { 100.f, 200.f };
	auto* sprite2 = registry.AddComponent<No::SpriteComponent>(light);
	sprite2->layer = 1;

	t2d2->scale = { 100.f, 100.f };
	sprite2->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/engine/Model/enemy.png");

	auto light2 = registry.GenerateEntity();
	auto* dir2 = registry.AddComponent<No::DirectionalLightComponent>(light2);
	dir2->color = { 1.f,1.f,1.f,1.f };
	dir2->direction = { 0.f,-1.f,0.f };
	dir2->intensity = 1.f;


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
