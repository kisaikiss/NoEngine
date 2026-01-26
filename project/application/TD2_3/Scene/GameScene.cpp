#include "GameScene.h"
#include "../Component/ColliderComponent.h"
#include "../Component/PhysicsComponent.h"
#include "../Component/BallStateComponent.h"
#include "../Component/RingAnimationComponent.h"
#include "../Component/VausStateComponent.h"

#include "../System/CollisionSystem.h"
//player
#include "../System/Player/BallControlSystem.h"
#include "../System/Player/VausControlSystem.h"

//ヨシダ追加しました。
#include"../System/Enemy/BossControlSystem.h"
//ヨシダ追加しました。
#include "../tag.h"

#include "engine/Functions/Renderer/Primitive.h"

using namespace NoEngine;

void GameScene::Setup()
{
	//player用システム
	AddSystem(std::make_unique<VausControlSystem>());
	AddSystem(std::make_unique<BallControlSystem>());

	AddSystem(std::make_unique<BossControlSystem>());
	//衝突判定用システム
	AddSystem(std::make_unique<CollisionSystem>());

	No::Registry& registry = *GetRegistry();
	InitVaus(registry);
	InitEnemy(registry);
	InitRing(registry);
	InitBall(registry);

	constexpr Vector3 kStartCameraPosition = Vector3{ 0.0f, 0.0f, -28.0f };
	//カメラ初期化
	camera_ = std::make_unique<NoEngine::Camera>();
	cameraTransform_.translate = kStartCameraPosition;
	camera_->SetTransform(cameraTransform_);
	SetCamera(camera_.get());
}

void GameScene::NotSystemUpdate()
{
#ifdef USE_IMGUI
	ImGui::Begin("camera");
	ImGui::DragFloat3("pos", &cameraTransform_.translate.x, 0.1f);
	ImGui::End();
	camera_->SetTransform(cameraTransform_);
#endif // USE_IMGUI

	camera_->Update();
	DestroyGameObject();
}

void GameScene::InitVaus(No::Registry& registry)
{
	No::Entity vausEntity = registry.GenerateEntity();
	registry.AddComponent<VausStateComponent>(vausEntity);
	auto* transform = registry.AddComponent<No::TransformComponent>(vausEntity);
	transform->translate = { 0.f, -4.85f, 0.f };

	auto* model = registry.AddComponent<No::MeshComponent>(vausEntity);
	NoEngine::ModelLoader::LoadModel("Vaus", "resources/engine/Model/testVaus.obj",model);

	auto* m = registry.AddComponent<No::MaterialComponent>(vausEntity);
	m->materials = NoEngine::ModelLoader::GetMaterial("Vaus");

	m->psoName = L"Renderer : Default PSO";
	m->psoId = NoEngine::Render::GetPSOID(m->psoName);
	m->rootSigId = NoEngine::Render::GetRootSignatureID(m->psoName);
}

void GameScene::InitRing(No::Registry& registry)
{
	No::Entity ringEntity = registry.GenerateEntity();
	registry.AddComponent<RingTag>(ringEntity);
	registry.AddComponent< RingAnimationComponent>(ringEntity);
	registry.AddComponent<No::TransformComponent>(ringEntity);
	auto* model = registry.AddComponent<No::MeshComponent>(ringEntity);
	NoEngine::ModelLoader::LoadModel("ring", "resources/engine/Model/testRing.obj", model);

	auto m = registry.AddComponent<No::MaterialComponent>(ringEntity);
	m->materials = NoEngine::ModelLoader::GetMaterial("ring");

	m->psoName = L"Renderer : Default PSO";
	m->psoId = NoEngine::Render::GetPSOID(m->psoName);
	m->rootSigId = NoEngine::Render::GetRootSignatureID(m->psoName);
}

void GameScene::InitBall(No::Registry& registry)
{
	No::Entity ballEntity = registry.GenerateEntity();
	registry.AddComponent<BallTag>(ballEntity);
	registry.AddComponent<PhysicsComponent>(ballEntity);
	registry.AddComponent<BallStateComponent>(ballEntity);
	auto* collider = registry.AddComponent<SphereColliderComponent>(ballEntity);
	collider->radius = 0.25f;
	collider->colliderType = ColliderMask::kBall;
	collider->collideMask = ColliderMask::kEnemy;

	registry.AddComponent<DeathFlag>(ballEntity);
	auto* transform = registry.AddComponent<No::TransformComponent>(ballEntity);
	transform->translate = { 0.0f, -4.35f, 0.f };

	auto* model = registry.AddComponent<No::MeshComponent>(ballEntity);
	NoEngine::ModelLoader::LoadModel("ball", "resources/engine/Model/ball.obj", model);

	auto m = registry.AddComponent<No::MaterialComponent>(ballEntity);
	m->materials = NoEngine::ModelLoader::GetMaterial("ball");

	m->psoName = L"Renderer : Default PSO";
	m->psoId = NoEngine::Render::GetPSOID(m->psoName);
	m->rootSigId = NoEngine::Render::GetRootSignatureID(m->psoName);
}

void GameScene::InitEnemy(No::Registry& registry)
{
	No::Entity enemyEntity = registry.GenerateEntity();
	registry.AddComponent<EnemyTag>(enemyEntity);
	auto* collider = registry.AddComponent<SphereColliderComponent>(enemyEntity);
	collider->colliderType = ColliderMask::kEnemy;
	collider->collideMask = ColliderMask::kBall;

	auto* transform = registry.AddComponent<No::TransformComponent>(enemyEntity);
	transform->rotation.FromAxisAngle(Vector3::UP, 3.14f);
	auto* model = registry.AddComponent<No::MeshComponent>(enemyEntity);
	NoEngine::ModelLoader::LoadModel("bat", "resources/engine/Model/bat/bat.obj", model);

	auto m = registry.AddComponent<No::MaterialComponent>(enemyEntity);
	m->materials = NoEngine::ModelLoader::GetMaterial("bat");
	m->materials[0].textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/engine/Model/bat/bat2.png");
	m->psoName = L"Renderer : Default PSO";
	m->psoId = NoEngine::Render::GetPSOID(m->psoName);
	m->rootSigId = NoEngine::Render::GetRootSignatureID(m->psoName);
}

void GameScene::InitBoss(No::Registry& registry)
{
	No::Entity bossEntity = registry.GenerateEntity();
	registry.AddComponent<Boss1Tag>(bossEntity);
	registry.AddComponent<SphereColliderComponent>(bossEntity);
	registry.AddComponent<No::TransformComponent>(bossEntity);
	auto* model = registry.AddComponent<No::MeshComponent>(bossEntity);
	NoEngine::ModelLoader::LoadModel("enemy", "resources/engine/Model/enemy.obj", model);

	auto m = registry.AddComponent<No::MaterialComponent>(bossEntity);
	m->materials = NoEngine::ModelLoader::GetMaterial("enemy");

	m->psoName = L"Renderer : Default PSO";
	m->psoId = NoEngine::Render::GetPSOID(m->psoName);
	m->rootSigId = NoEngine::Render::GetRootSignatureID(m->psoName);
}

void GameScene::DestroyGameObject()
{
	No::Registry& registry = *GetRegistry();

	auto view = registry.View<DeathFlag>();
	for (auto entity : view)
	{
		if (registry.Has<DeathFlag>(entity))
		{
			auto* flag = registry.GetComponent<DeathFlag>(entity);
			if (flag->isDead)
			{
				registry.DestroyEntity(entity);
			}
		}
	}
}
