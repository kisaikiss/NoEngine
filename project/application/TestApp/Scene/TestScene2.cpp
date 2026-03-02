#include "stdafx.h"
#include "TestScene2.h"

void TestScene2::Setup() {
	AddSystem(std::make_unique<No::AnimationSystem>());
	AddSystem(std::make_unique<No::EditSystem>());

	No::Registry& registry = *GetRegistry();
	No::Entity entity = registry.GenerateEntity();

	auto* model = registry.AddComponent<No::MeshComponent>(entity);
	auto* t = registry.AddComponent<No::TransformComponent>(entity);
	auto* imguiName = registry.AddComponent<No::EditTag>(entity);
	imguiName->name = "girl";
	t->rotation.FromAxisAngle(No::Vector3(0.f, 1.f, 0.f), PI);
	t->translate = { 0.f,-1.5f,4.f };
	auto* m = registry.AddComponent<No::MaterialComponent>(entity);
	auto* a = registry.AddComponent<No::AnimatorComponent>(entity);
	No::ModelLoader::LoadModel("magiclash", "resources/engine/Model/test/TD_girl/test7.gltf");
	No::ModelLoader::GetModel("magiclash", model, a);
	m->materials = No::ModelLoader::GetMaterial("magiclash");
	m->drawOutline = true;
	m->enableSkinning = true;

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

void TestScene2::NotSystemUpdate() {
#ifdef USE_IMGUI
	ImGui::Begin("camera");
	ImGui::DragFloat3("pos", &cameraTransform_.translate.x, 0.1f);
	ImGui::End();
	camera_->SetTransform(cameraTransform_);
#endif // USE_IMGUI

	camera_->Update();
}
