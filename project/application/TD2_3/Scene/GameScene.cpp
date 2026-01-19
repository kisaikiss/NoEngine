#include "GameScene.h"
#include "../Component/ColliderComponent.h"

#include "../System/CollisionSystem.h"
#include "../System/PlayerControlSystem.h"
#include "../tag.h"

void GameScene::Setup()
{
	AddSystem(std::make_unique<PlayerControlSystem>());
	AddSystem(std::make_unique<CollisionSystem>());

	No::Registry& registry = *GetRegistry();
	InitPlayer(registry);
    InitEnemy(registry);
	


	camera_ = std::make_unique<NoEngine::Camera>();
	cameraTransform_.translate.z = -10.f;
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
}

void GameScene::InitPlayer(No::Registry& registry)
{
	No::Entity playerEntity = registry.GenerateEntity();
	registry.AddComponent<PlayerTag>(playerEntity);
	registry.AddComponent<SphereColliderComponent>(playerEntity);
	registry.AddComponent<No::TransformComponent>(playerEntity)->rotation.FromAxisAngle(NoEngine::Vector3::UP, 3.14f);
	auto* model = registry.AddComponent<No::MeshComponent>(playerEntity);
	model->mesh = NoEngine::ModelLoader::LoadModel("enemy", "resources/engine/Model/enemy.obj");

	auto m = registry.AddComponent<No::MaterialComponent>(playerEntity);
	m->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/engine/Model/enemy.png");
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
