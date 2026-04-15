#include "TestScene.h"

void TestScene::Setup() {
	AddSystems();

	auto& registry = *GetRegistry();

	// プレイヤー
	No::Entity player = registry.GenerateEntity();
	auto* model = registry.AddComponent<No::MeshComponent>(player);
	auto* t = registry.AddComponent<No::TransformComponent>(player);
	auto* imguiName = registry.AddComponent<No::EditTag>(player);
	imguiName->name = "player";
	t->rotation.FromAxisAngle(No::Vector3(0.f, 1.f, 0.f), PI);
	t->translate = { 0.f,-1.5f,4.f };
	auto* m = registry.AddComponent<No::MaterialComponent>(player);
	auto* a = registry.AddComponent<No::AnimatorComponent>(player);
	No::ModelLoader::LoadModel("magiclash", "resources/engine/Model/test/TD_girl/test7.gltf");
	No::ModelLoader::GetModel("magiclash", model, a);
	m->materials = No::ModelLoader::GetMaterial("magiclash");
	m->drawOutline = true;
	m->enableSkinning = true;
	m->psoName = L"Renderer : DefaultSkinned PSO";
	m->psoId = NoEngine::Render::GetPSOID(m->psoName);
	m->rootSigId = NoEngine::Render::GetRootSignatureID(m->psoName);

	// カメラ
	auto camera = registry.GenerateEntity();
	registry.AddComponent<No::ActiveCameraTag>(camera);
	registry.AddComponent<No::CameraComponent>(camera);
	registry.AddComponent<No::DebugCameraComponent>(camera);
	auto* cameraEditTag = registry.AddComponent<No::EditTag>(camera);
	cameraEditTag->name = "camera";
	auto* cameraTransform = registry.AddComponent<No::TransformComponent>(camera);
	cameraTransform->translate.z = -5.f;
}

void TestScene::NotSystemUpdate() {
}

void TestScene::AddSystems() {
	AddSystem(std::make_unique<No::AnimationSystem>());
	AddSystem(std::make_unique<No::SpriteAnimationSystem>());
	AddSystem(std::make_unique<No::EditSystem>());
	AddSystem(std::make_unique<No::DebugCameraSystem>());
	AddSystem(std::make_unique<No::CameraSystem>());
}
