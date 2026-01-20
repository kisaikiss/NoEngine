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
	AddSystem(std::make_unique<BallControlSystem>());
	AddSystem(std::make_unique<VausControlSystem>());

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
	registry.AddComponent<VausTag>(vausEntity);
	auto* transform = registry.AddComponent<No::TransformComponent>(vausEntity);
	transform->translate = { 0.f, -4.85f, 0.f };

	auto* model = registry.AddComponent<No::MeshComponent>(vausEntity);
	model->mesh = NoEngine::ModelLoader::LoadModel("Vaus", "resources/engine/Model/testVaus.obj");

	auto* m = registry.AddComponent<No::MaterialComponent>(vausEntity);
	m->color = Color(0xcc0000ff);
	m->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/engine/white1x1.png");
	m->pso = &NoEngine::Render::GetPSO(L"Renderer : Default PSO");
}

void GameScene::InitRing(No::Registry& registry)
{
	No::Entity ringEntity = registry.GenerateEntity();
	registry.AddComponent<RingTag>(ringEntity);
	registry.AddComponent< RingAnimationComponent>(ringEntity)->targetScale = Vector3(1.2f, 1.2f, 1.2f);
	registry.AddComponent<No::TransformComponent>(ringEntity);
	auto* model = registry.AddComponent<No::MeshComponent>(ringEntity);
	model->mesh = NoEngine::ModelLoader::LoadModel("ring", "resources/engine/Model/testRing.obj");

	auto m = registry.AddComponent<No::MaterialComponent>(ringEntity);
	m->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/engine/white1x1.png");
	m->pso = &NoEngine::Render::GetPSO(L"Renderer : Default PSO");
}

void GameScene::InitBall(No::Registry& registry)
{
	No::Entity ballEntity = registry.GenerateEntity();
	registry.AddComponent<BallTag>(ballEntity);
	registry.AddComponent<PhysicsComponent>(ballEntity);
	registry.AddComponent<BallStateComponent>(ballEntity);
	auto* collider = registry.AddComponent<SphereColliderComponent>(ballEntity);
	collider->radius = 0.25f;

	registry.AddComponent<DeathFlag>(ballEntity);
	auto* transform = registry.AddComponent<No::TransformComponent>(ballEntity);
	transform->translate = { 0.0f, -4.35f, 0.f };

	auto* model = registry.AddComponent<No::MeshComponent>(ballEntity);
	model->mesh = NoEngine::ModelLoader::LoadModel("ball", "resources/engine/Model/ball.obj");

	auto m = registry.AddComponent<No::MaterialComponent>(ballEntity);
	m->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/engine/white1x1.png");
	m->pso = &NoEngine::Render::GetPSO(L"Renderer : Default PSO");

}

void GameScene::InitEnemy(No::Registry& registry)
{
	No::Entity enemyEntity = registry.GenerateEntity();
	registry.AddComponent<EnemyTag>(enemyEntity);
	registry.AddComponent<SphereColliderComponent>(enemyEntity);
	registry.AddComponent<No::TransformComponent>(enemyEntity);
	auto* model = registry.AddComponent<No::MeshComponent>(enemyEntity);
	model->mesh = NoEngine::ModelLoader::LoadModel("enemy", "resources/engine/Model/enemy.obj");

	auto m = registry.AddComponent<No::MaterialComponent>(enemyEntity);
	m->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/engine/uvChecker.png");
	m->pso = &NoEngine::Render::GetPSO(L"Renderer : Default PSO");
}

void GameScene::InitBoss(No::Registry& registry)
{
	No::Entity bossEntity = registry.GenerateEntity();
	registry.AddComponent<Boss1Tag>(bossEntity);
	registry.AddComponent<SphereColliderComponent>(bossEntity);
	registry.AddComponent<No::TransformComponent>(bossEntity);
	auto* model = registry.AddComponent<No::MeshComponent>(bossEntity);
	model->mesh = NoEngine::ModelLoader::LoadModel("enemy", "resources/engine/Model/enemy.obj");

	auto m = registry.AddComponent<No::MaterialComponent>(bossEntity);
	m->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/engine/uvChecker.png");
	m->pso = &NoEngine::Render::GetPSO(L"Renderer : Default PSO");
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
