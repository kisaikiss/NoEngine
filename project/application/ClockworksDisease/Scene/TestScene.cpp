#include "TestScene.h"

#include "../System/Player/PlayerMoveSystem.h"
#include "../System/Camera/FollowCameraSystem.h"
#include "../System/Game/CollisionEventSystem.h"
#include "../System/Player/PlayerPushBackSystem.h"
#include "../System/Object/BoxColliderUpdateSystem.h"

#include "../Component/Player/PlayerComponent.h"
#include "../Component/Camera/FollowCameraComponent.h"
#include "../System/Game/CollisionLayer.h"

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
		//auto* a = registry.AddComponent<No::AnimatorComponent>(player);
		//No::ModelLoader::LoadModel("magiclash", "resources/game/ClockworksDisease/Model/player/magiclash2.gltf");
		No::ModelLoader::LoadModel("magiclash", "resources/engine/Model/enemy.obj");
		No::ModelLoader::GetModel("magiclash", model);
		m->materials = No::ModelLoader::GetMaterial("magiclash");
		m->drawOutline = false;
		m->enableSkinning = false;
		m->psoName = L"Renderer : Toon PSO";
		m->psoId = NoEngine::Render::GetPSOID(m->psoName);
		m->rootSigId = NoEngine::Render::GetRootSignatureID(m->psoName);

		auto* collider = registry.AddComponent<No::CapsuleCollider>(player);
		collider->radius = 0.25f;
		collider->localP0.y = 0.25f;
		collider->localP1.y = 1.25f;

		registry.AddComponent<No::GroundStateComponent>(player);
		registry.AddComponent<No::CollisionBody>(player)->type = No::BodyType::Dynamic;

		registry.AddComponent<PlayerComponent>(player);
		registry.AddComponent<No::VelocityComponent>(player);
		registry.AddComponent<CollisionLayerComponent>(player)->layer = CollisionLayerComponent::Player;
	}

	// 箱
	{
		No::Entity box = registry.GenerateEntity();
		auto* bm = registry.AddComponent<No::MeshComponent>(box);
		No::ModelLoader::LoadModel("box", "resources/engine/Model/block/block.obj");
		No::ModelLoader::GetModel("box", bm);
		auto* bmm = registry.AddComponent<No::MaterialComponent>(box);
		bmm->materials = No::ModelLoader::GetMaterial("box");
		bmm->psoName = L"Renderer : Default PSO";
		bmm->psoId = NoEngine::Render::GetPSOID(bmm->psoName);
		bmm->rootSigId = NoEngine::Render::GetRootSignatureID(bmm->psoName);
		auto* boxT = registry.AddComponent<No::TransformComponent>(box);
		boxT->translate.x = 2.f;
		boxT->translate.y = 0.5f;
		registry.AddComponent<No::EditTag>(box)->name = "box";
		auto* collider = registry.AddComponent<No::AABBCollider>(box);
		collider->max = 0.5f;
		collider->min = -0.5f;
		registry.AddComponent<CollisionLayerComponent>(box)->layer = CollisionLayerComponent::Terrain;
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
	AddSystem(std::make_unique<BoxColliderUpdateSystem>());
	AddSystem(std::make_unique<PlayerMoveSystem>());
	AddSystem(std::make_unique<FollowCameraSystem>());

	AddSystem(std::make_unique<No::NarrowPhaseSystem>());
	AddSystem(std::make_unique<No::CollisionResolutionSystem>());
	AddSystem(std::make_unique<CollisionEventSystem>());
	AddSystem(std::make_unique<PlayerPushBackSystem>());
	AddSystem(std::make_unique<No::MovementSystem>());
	AddSystem(std::make_unique<No::AnimationSystem>());
	AddSystem(std::make_unique<No::SpriteAnimationSystem>());
	AddSystem(std::make_unique<No::EditSystem>());
	AddSystem(std::make_unique<No::DebugCameraSystem>());
	AddSystem(std::make_unique<No::CameraSystem>());
}
