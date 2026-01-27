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
	NoEngine::ModelLoader::LoadModel("magiclash", "resources/engine/Model/test/TD_girl/test7.gltf");
	NoEngine::ModelLoader::GetModel("magiclash", model, a);
	m->materials = NoEngine::ModelLoader::GetMaterial("magiclash");
	
	m->psoName = L"Renderer : DefaultSkinned PSO";
	m->psoId = NoEngine::Render::GetPSOID(m->psoName);
	m->rootSigId = NoEngine::Render::GetRootSignatureID(m->psoName);


	auto* t2d = registry.AddComponent<No::Transform2DComponent>(entity);
	t2d->translate = { 100.f, 200.f };
	auto* sprite = registry.AddComponent<No::SpriteComponent>(entity);
	sprite->layer = 1;

	t2d->scale = { 100.f, 100.f };
	sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/engine/Model/enemy.png");
	
	for (uint32_t i = 0; i < 10; i++) {
		auto e = registry.GenerateEntity();
		auto* transform = registry.AddComponent<No::Transform2DComponent>(e);
		transform->translate = { static_cast<float>(i)*100.f, static_cast<float>(i + 1) * 10.f };
		transform->scale = { 50.f, 50.f };
		auto* sp = registry.AddComponent<No::SpriteComponent>(e);
		if (i == 5) {
			sp->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/engine/uvChecker.png");
			sp->layer = 2;
		} else if (i == 7) {
			sp->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/engine/Model/enemy.png");
			sp->layer = 0;
		} else {
			sp->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/engine/flower.png");
			sp->layer = 1;
		}
	}

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
