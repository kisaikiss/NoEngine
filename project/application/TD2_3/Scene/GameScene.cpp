#include "GameScene.h"
#include "../Component/ColliderComponent.h"
#include "../Component/PhysicsComponent.h"
#include "../Component/BallStateComponent.h"
#include "../Component/RingAnimationComponent.h"
#include "../Component/VausStateComponent.h"
#include "../Component/BackGroundComponent.h"
#include"../Component/NormalEnemyComponent.h"
//collision
#include "../System/CollisionSystem.h"
//player
#include "../System/Player/BallControlSystem.h"
#include "../System/Player/VausControlSystem.h"

//effect
#include "../System/BackGroundEffectSystem.h"
//ヨシダ追加しました。
//Enemy
#include "../System/Enemy/BossControlSystem.h"
#include"../System/Enemy/NormalEnemyControlSystem.h"
//Human
#include "../System/Human/BatGirlControlSystem.h"
#include "../System/Human/PlayerGirlControlSystem.h"

//ヨシダ追加しました。
#include "../tag.h"

#include "engine/Functions/Renderer/Primitive.h"

using namespace NoEngine;

void GameScene::Setup()
{
	//アニメーションシステム
	AddSystem(std::make_unique<No::AnimationSystem>());
	//effect
	AddSystem(std::make_unique<BackGroundEffectSystem>());


	//player用システム
	AddSystem(std::make_unique<VausControlSystem>());
    AddSystem(std::make_unique<BallControlSystem>());
    //Enemy
    AddSystem(std::make_unique<NormalEnemyControlSystem>());
	AddSystem(std::make_unique<BossControlSystem>());
	//こうもり少女のシステム
	AddSystem(std::make_unique<BatGirlControlSystem>());
	//プレイヤー少女システム
	AddSystem(std::make_unique<PlayerGirlControlSystem>());


	//衝突判定用システム
	AddSystem(std::make_unique<CollisionSystem>());

	No::Registry& registry = *GetRegistry();
	InitBackGround(registry);
	InitVaus(registry);
	InitEnemy(registry);
	InitRing(registry);
	InitBall(registry);
	InitBoss(registry);
	InitBatGirl(registry);
	InitPlayerGirl(registry);
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
    registry.AddComponent<VausStateComponent>(vausEntity);
    auto* transform = registry.AddComponent<No::TransformComponent>(vausEntity);
    transform->translate = { 0.f, -4.85f, 0.f };

    auto* model = registry.AddComponent<No::MeshComponent>(vausEntity);
    NoEngine::ModelLoader::LoadModel("Vaus", "resources/engine/Model/testVaus.obj", model);

	auto* m = registry.AddComponent<No::MaterialComponent>(vausEntity);
	m->materials = NoEngine::ModelLoader::GetMaterial("Vaus");
	m->materials.front().color = {0.8f,0.2f,0.2f};

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
	m->materials.front().color.a = 0.7f;
	m->materials.front().color.b = 0.5f;

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
    registry.AddComponent<NormalEnemyTag>(enemyEntity);
    registry.AddComponent<DeathFlag>(enemyEntity);
    auto* enemy = registry.AddComponent<NormalEnemyComponent>(enemyEntity);
    enemy->velocity = { 0.5f,0.5f,0.0f };
    enemy->hp = 100;
    auto* collider = registry.AddComponent<SphereColliderComponent>(enemyEntity);
    collider->colliderType = ColliderMask::kEnemy;
    collider->collideMask = ColliderMask::kBall;
    collider->radius = 0.5f;
    auto* transform = registry.AddComponent<No::TransformComponent>(enemyEntity);
    transform->rotation.FromAxisAngle(Vector3::UP, 3.14f);
    //transform->translate = GenerateRandomPointInCircle(5.0f); // 半径5の円内に配置
    transform->translate = { 0.0f, -4.35f, 0.0f }; // Ballと同じ位置にしてみよう！
    auto* model = registry.AddComponent<No::MeshComponent>(enemyEntity);
    NoEngine::ModelLoader::LoadModel("bat", "resources/game/td_2304/Model/bat/bat.obj", model);

    auto m = registry.AddComponent<No::MaterialComponent>(enemyEntity);
    m->materials = NoEngine::ModelLoader::GetMaterial("bat");
    m->materials[0].textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Model/bat/bat2.png");
    m->psoName = L"Renderer : Default PSO";
    m->psoId = NoEngine::Render::GetPSOID(m->psoName);
    m->rootSigId = NoEngine::Render::GetRootSignatureID(m->psoName);

}

void GameScene::InitBoss(No::Registry& registry)
{

    No::Entity bossEntity = registry.GenerateEntity();
    registry.AddComponent<Boss1Tag>(bossEntity);
    registry.AddComponent<DeathFlag>(bossEntity);
    auto* collider = registry.AddComponent<SphereColliderComponent>(bossEntity);
    collider->colliderType = ColliderMask::kEnemy;
    collider->collideMask = ColliderMask::kBall;

    auto* transform = registry.AddComponent<No::TransformComponent>(bossEntity);
    transform->rotation.FromAxisAngle(Vector3::UP, 3.14f);


    auto* model = registry.AddComponent<No::MeshComponent>(bossEntity);
    auto* animationComp = registry.AddComponent<No::AnimatorComponent>(bossEntity);
    NoEngine::ModelLoader::LoadModel("batBoss", "resources/game/td_2304/Model/batBoss/batBoss.gltf", model, animationComp);

    auto m = registry.AddComponent<No::MaterialComponent>(bossEntity);
    m->materials = NoEngine::ModelLoader::GetMaterial("batBoss");

    m->psoName = L"Renderer : DefaultSkinned PSO";
    m->psoId = NoEngine::Render::GetPSOID(m->psoName);
    m->rootSigId = NoEngine::Render::GetRootSignatureID(m->psoName);


}

void GameScene::InitBackGround(No::Registry& registry)
{
    No::Entity backGroundEntity = registry.GenerateEntity();
    auto* transform = registry.AddComponent<No::TransformComponent>(backGroundEntity);
    transform->translate.z = 5;
    transform->scale = { 30,30,1 };

    registry.AddComponent<BackGroundComponent>(backGroundEntity);
}

void GameScene::InitBatGirl(No::Registry& registry)
{
    No::Entity batGirlEntity = registry.GenerateEntity();
    registry.AddComponent<BatGirlTag>(batGirlEntity);

    auto* transform = registry.AddComponent<No::TransformComponent>(batGirlEntity);
    auto* model = registry.AddComponent<No::MeshComponent>(batGirlEntity);
    auto* animationComp = registry.AddComponent<No::AnimatorComponent>(batGirlEntity);
    NoEngine::ModelLoader::LoadModel("batGirl", "resources/game/td_2304/Model/batGirl/batGirl.gltf", model, animationComp);

    transform->translate = { 6.1f,-14.55f,-8.5f };
    transform->rotation.FromAxisAngle(NoEngine::Vector3::UP, 3.14f);

    auto m = registry.AddComponent<No::MaterialComponent>(batGirlEntity);
    m->materials = NoEngine::ModelLoader::GetMaterial("batGirl");

    m->psoName = L"Renderer : DefaultSkinned PSO";
    m->psoId = NoEngine::Render::GetPSOID(m->psoName);
    m->rootSigId = NoEngine::Render::GetRootSignatureID(m->psoName);
}

void GameScene::InitPlayerGirl(No::Registry& registry)
{
    No::Entity playerGirlEntity = registry.GenerateEntity();
    registry.AddComponent<PlayerGirlTag>(playerGirlEntity);

    auto* transform = registry.AddComponent<No::TransformComponent>(playerGirlEntity);
    auto* model = registry.AddComponent<No::MeshComponent>(playerGirlEntity);
    auto* animationComp = registry.AddComponent<No::AnimatorComponent>(playerGirlEntity);
    NoEngine::ModelLoader::LoadModel("playerGirl", "resources/game/td_2304/Model/playerGirl/playerGirl.gltf", model, animationComp);

    transform->translate = { -6.1f,-14.55f,-8.5f };
    transform->rotation.FromAxisAngle(NoEngine::Vector3::UP, 3.14f);

    auto m = registry.AddComponent<No::MaterialComponent>(playerGirlEntity);
    m->materials = NoEngine::ModelLoader::GetMaterial("playerGirl");

    m->psoName = L"Renderer : DefaultSkinned PSO";
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

NoEngine::Vector3 GameScene::GenerateRandomPointInCircle(float radius)
{

   int randomAngle =  rand() % 628;
   float angle = randomAngle * 0.01f;

   // 0〜1の乱数を生成して√で均等分布に 
  float raw = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
  float r = radius * std::sqrt(raw);

    float x = r * std::cos(angle);
    float y = r * std::sin(angle);

    return Vector3{ x, y, -0.5f }; // Zは0で平面上に配置
}
