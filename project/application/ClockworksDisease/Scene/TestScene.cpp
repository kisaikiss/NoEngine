#include "TestScene.h"

#include "../System/PlayerMoveSystem.h"
#include "../System/Camera/FollowCameraSystem.h"

#include "../Component/Player/PlayerComponent.h"
#include "../Component/Camera/FollowCameraComponent.h"

void TestScene::Setup() {
	AddSystems();

	auto& registry = *GetRegistry();

	// プレイヤー
	{
		No::Entity player = registry.GenerateEntity();
		auto* model = registry.AddComponent<No::MeshComponent>(player);
		auto* t = registry.AddComponent<No::TransformComponent>(player);
		auto* imguiName = registry.AddComponent<No::EditTag>(player);
		imguiName->name = "player";
		t->rotation.FromAxisAngle(No::Vector3(0.f, 1.f, 0.f), PI);
		t->translate = { 0.f, -0.f, 0.f };
		auto* m = registry.AddComponent<No::MaterialComponent>(player);
		auto* a = registry.AddComponent<No::AnimatorComponent>(player);
		No::ModelLoader::LoadModel("magiclash", "resources/game/ClockworksDisease/Model/player/magiclash2.gltf");
		No::ModelLoader::GetModel("magiclash", model, a);
		m->materials = No::ModelLoader::GetMaterial("magiclash");
		m->drawOutline = true;
		m->enableSkinning = true;
		m->psoName = L"Renderer : ToonSkinned PSO";
		m->psoId = NoEngine::Render::GetPSOID(m->psoName);
		m->rootSigId = NoEngine::Render::GetRootSignatureID(m->psoName);
		
		registry.AddComponent<PlayerComponent>(player);
		registry.AddComponent<No::VelocityComponent>(player);
	}

	// 地面
	{
		No::Entity background = registry.GenerateEntity();
		auto* bm = registry.AddComponent<No::MeshComponent>(background);
		No::ModelLoader::LoadModel("background", "resources/game/ClockworksDisease/Model/ground/ground.obj");
		No::ModelLoader::GetModel("background", bm);
		auto* bmm = registry.AddComponent<No::MaterialComponent>(background);
		bmm->materials = No::ModelLoader::GetMaterial("background");
		bmm->psoName = L"Renderer : Default PSO";
		bmm->psoId = NoEngine::Render::GetPSOID(bmm->psoName);
		bmm->rootSigId = NoEngine::Render::GetRootSignatureID(bmm->psoName);
		registry.AddComponent<No::TransformComponent>(background);
		auto* backgroundTag = registry.AddComponent<No::EditTag>(background);
		backgroundTag->name = "background";
	}


	// カメラ
	auto camera = registry.GenerateEntity();
	registry.AddComponent<No::ActiveCameraTag>(camera);
	registry.AddComponent<No::CameraComponent>(camera);
	//registry.AddComponent<No::DebugCameraComponent>(camera);
	registry.AddComponent<FollowCameraComponent>(camera);
	auto* cameraEditTag = registry.AddComponent<No::EditTag>(camera);
	cameraEditTag->name = "camera";
	registry.AddComponent<No::TransformComponent>(camera);

	// 方向ライト
	auto directionalLight = registry.GenerateEntity();
	auto* dir = registry.AddComponent<No::DirectionalLightComponent>(directionalLight);
	dir->color = No::Color::WHITE;
	dir->direction = { 0.f,-1.f,0.f };
	dir->intensity = 1.0f;
	auto* lightTag = registry.AddComponent<No::EditTag>(directionalLight);
	lightTag->name = "directionalLight";
}

void TestScene::NotSystemUpdate() {
}

void TestScene::AddSystems() {
	AddSystem(std::make_unique<PlayerMoveSystem>());
	AddSystem(std::make_unique<FollowCameraSystem>());

	AddSystem(std::make_unique<No::MovementSystem>());
	AddSystem(std::make_unique<No::AnimationSystem>());
	AddSystem(std::make_unique<No::SpriteAnimationSystem>());
	AddSystem(std::make_unique<No::EditSystem>());
	AddSystem(std::make_unique<No::DebugCameraSystem>());
	AddSystem(std::make_unique<No::CameraSystem>());
}
