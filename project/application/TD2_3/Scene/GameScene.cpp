#include "GameScene.h"
#include "../Component/ColliderComponent.h"
#include "../Component/PhysicsComponent.h"
#include "../Component/BallStateComponent.h"
#include "../Component/RingAnimationComponent.h"
#include "../Component/VausStateComponent.h"
#include "../Component/BackGroundComponent.h"
#include"../Component/NormalEnemyComponent.h"
#include"../Component/BatBossComponent.h"
#include "../Component/BallTrailComponent.h"
#include "../Component/PlayerstatusComponent.h"
#include "../Component/UpgradeChooseComponent.h"
//collision
#include "../System/CollisionSystem.h"
//player
#include "../System/Player/BallControlSystem.h"
#include "../System/Player/VausControlSystem.h"
#include "../System/Player/PlayerStatusSystem.h"
#include "../System/Player/UpgradeSelectionSystem.h"

//effect
#include "../System/BackGroundEffectSystem.h"
//ヨシダ追加しました。
//Enemy
#include "../System/Enemy/Boss/BossControlSystem.h"
#include"../System/Enemy/NormalEnemy/NormalEnemyControlSystem.h"
//Human
#include "../System/Human/BatGirlControlSystem.h"
#include "../System/Human/PlayerGirlControlSystem.h"

#include"../System/HpSpriteControlSystem.h"
#include"../SpriteConfigManager/SpriteConfigManager.h"
//ヨシダ追加しました。

#include "../tag.h"


using namespace NoEngine;

void GameScene::Setup()
{
    //乱数の初期化
    srand(static_cast<unsigned int>(time(nullptr)));

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
    //playerステータス管理システム
    AddSystem(std::make_unique<PlayerStatusSystem>());
    AddSystem(std::make_unique<UpgradeSelectionSystem>());
    //HISprite
    AddSystem(std::make_unique<HpSpriteControlSystem>());
    //衝突判定用システム
    AddSystem(std::make_unique<CollisionSystem>());


    SpriteConfigManager::Get().Load("resources/game/td_2304/Json/tdSpriteConfig.json");

    No::Registry& registry = *GetRegistry();
    InitBackGround(registry);
    InitVaus(registry);
    InitEnemy(registry);
    InitRing(registry);
    InitBall(registry);
    InitBoss(registry);
    InitBatGirl(registry);
    InitPlayerGirl(registry);
    InitPlayerStatus(registry);
    InitLights(registry);
    InitHpGaugeSprite(registry);
    InitLevelGaugeSprite(registry);
    InitChooseSprite(registry);

    constexpr Vector3 kStartCameraPosition = Vector3{ 0.0f, 0.0f, -28.0f };
    //カメラ初期化
    camera_ = std::make_unique<NoEngine::Camera>();
    cameraTransform_.translate = kStartCameraPosition;
    camera_->SetTransform(cameraTransform_);
    SetCamera(camera_.get());

    SoundLoad();
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
    NoEngine::ModelLoader::LoadModel("paddleMiddle", "resources/game/td_2304/Model/paddle/middle1.obj", model);

    auto* m = registry.AddComponent<No::MaterialComponent>(vausEntity);
    m->materials = NoEngine::ModelLoader::GetMaterial("paddleMiddle");
    m->color = 0xA82C57ff;

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
    NoEngine::ModelLoader::LoadModel("circle", "resources/game/td_2304/Model/circle/circle.obj", model);

    auto m = registry.AddComponent<No::MaterialComponent>(ringEntity);
    m->materials = NoEngine::ModelLoader::GetMaterial("circle");
    m->color.a = 0.7f;
    m->color.b = 0.5f;

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

    //軌跡
    auto* trailComp = registry.AddComponent<BallTrailComponent>(ballEntity);
    // 必要ならパラメータを微調整
    trailComp->maxAge = 0.6f;
    trailComp->sampleInterval = 0.02f;
    trailComp->thickness = 0.35f;
    trailComp->maxSamples = 256;
    auto* collider = registry.AddComponent<SphereColliderComponent>(ballEntity);
    collider->radius = 0.25f;
    collider->colliderType = ColliderMask::kBall;
    collider->collideMask = ColliderMask::kEnemy;

    registry.AddComponent<DeathFlag>(ballEntity);
    auto* transform = registry.AddComponent<No::TransformComponent>(ballEntity);
    transform->translate = { 0.0f, -4.35f, 0.f };

    auto* model = registry.AddComponent<No::MeshComponent>(ballEntity);
    NoEngine::ModelLoader::LoadModel("ball", "resources/game/td_2304/Model/ball/ball.obj", model);

    auto m = registry.AddComponent<No::MaterialComponent>(ballEntity);
    m->materials = NoEngine::ModelLoader::GetMaterial("ball");

    m->psoName = L"Renderer : Default PSO";
    m->psoId = NoEngine::Render::GetPSOID(m->psoName);
    m->rootSigId = NoEngine::Render::GetRootSignatureID(m->psoName);
}

void GameScene::InitEnemy(No::Registry& registry)
{
    for (int i = 0; i < 5; ++i)
    {
        No::Entity enemyEntity = registry.GenerateEntity();
        registry.AddComponent<NormalEnemyTag>(enemyEntity);
        registry.AddComponent<DeathFlag>(enemyEntity);

        auto* enemy = registry.AddComponent<NormalEnemyComponent>(enemyEntity);
        //enemy->velocity = { 0.5f,0.5f,0.0f };
        //enemy->stateManager->Start(enemy);
        //enemy->stateManager->ChangeState<EnemyAppear<NormalEnemyComponent>>(registry);

        auto* collider = registry.AddComponent<SphereColliderComponent>(enemyEntity);
        collider->colliderType = ColliderMask::kEnemy;
        collider->collideMask = ColliderMask::kBall;

        auto* transform = registry.AddComponent<No::TransformComponent>(enemyEntity);
        transform->rotation.FromAxisAngle(Vector3::UP, 3.14f);
        transform->translate = GenerateRandomPointInCircle(2.0f, 3.0f);

        enemy->defaultTranslate_ = transform->translate;

        auto* model = registry.AddComponent<No::MeshComponent>(enemyEntity);
        auto* animationComp = registry.AddComponent<No::AnimatorComponent>(enemyEntity);
        NoEngine::ModelLoader::LoadModel(enemyResources_.modelName, enemyResources_.modelPath, model, animationComp);

        auto m = registry.AddComponent<No::MaterialComponent>(enemyEntity);
        m->materials = NoEngine::ModelLoader::GetMaterial("bat");
        m->materials[0].textureHandle = NoEngine::TextureManager::LoadCovertTexture(enemyResources_.texturePath);
        m->psoName = L"Renderer : Default PSO";
        m->psoId = NoEngine::Render::GetPSOID(m->psoName);
        m->rootSigId = NoEngine::Render::GetRootSignatureID(m->psoName);
    }
}

void GameScene::InitBoss(No::Registry& registry)
{

    No::Entity bossEntity = registry.GenerateEntity();
    registry.AddComponent<Boss1Tag>(bossEntity);
    registry.AddComponent<DeathFlag>(bossEntity);
    registry.AddComponent<BattBossComponent>(bossEntity);
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
    m->enableSkinning = true;
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

	m->psoName = L"Renderer : ToonSkinned PSO";
	m->enableSkinning = true;
	m->drawOutline = true;
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

	m->psoName = L"Renderer : ToonSkinned PSO";
	m->enableSkinning = true;
	m->drawOutline = true;
	m->psoId = NoEngine::Render::GetPSOID(m->psoName);
	m->rootSigId = NoEngine::Render::GetRootSignatureID(m->psoName);
}

void GameScene::InitPlayerStatus(No::Registry& registry)
{
    No::Entity playerStatusEntity = registry.GenerateEntity();
    auto* status = registry.AddComponent<PlayerStatusComponent>(playerStatusEntity);
    status->exp = 0;
    status->hp = 5;
    status->level = 1;
    status->score = 0;

    //No::Entity upgradeEntity = registry.GenerateEntity();
    //registry.AddComponent<UpgradeChooseComponent>(upgradeEntity);
}

void GameScene::InitLights(No::Registry& registry)
{
    auto light = registry.GenerateEntity();
    auto* dir = registry.AddComponent<No::DirectionalLightComponent>(light);
    dir->color = { 1.f,1.f,1.f,1.f };
    dir->direction = { 0.f,-1.f,0.f };
    dir->intensity = 1.f;
}

void GameScene::InitHpGaugeSprite(No::Registry& registry)
{
    CreateSprite(registry, "hp.png", "PlayerHpGauge");
}

void GameScene::InitLevelGaugeSprite(No::Registry& registry)
{
    CreateSprite(registry, "lv.png", "Level");
}

void GameScene::InitChooseSprite(No::Registry& registry)
{
    //hp上限up
    {
        No::Entity hpLimitUpEntity = registry.GenerateEntity();
        registry.AddComponent<StatusSpriteTag>(hpLimitUpEntity);
        auto* ts = registry.AddComponent<No::Transform2DComponent>(hpLimitUpEntity);
        ts->scale = { 320,144 };
        auto* sprite = registry.AddComponent<No::SpriteComponent>(hpLimitUpEntity);
        sprite->name = "hpLimitUp";
        sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Sprite/hpLimitUp.png");
    }

    //ballUp
    {
        No::Entity ballUpEntity = registry.GenerateEntity();
        registry.AddComponent<StatusSpriteTag>(ballUpEntity);
        auto* ts = registry.AddComponent<No::Transform2DComponent>(ballUpEntity);
        ts->scale = { 320,144 };
        auto* sprite = registry.AddComponent<No::SpriteComponent>(ballUpEntity);
        sprite->name = "ballUp";
        sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Sprite/ballUp.png");
    }

    //paddleSpread
    {
        No::Entity paddleSpreadEntity = registry.GenerateEntity();
        registry.AddComponent<StatusSpriteTag>(paddleSpreadEntity);
        auto* ts = registry.AddComponent<No::Transform2DComponent>(paddleSpreadEntity);
        ts->scale = { 320,144 };
        auto* sprite = registry.AddComponent<No::SpriteComponent>(paddleSpreadEntity);
        sprite->name = "paddleSpread";
        sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Sprite/paddleSpread.png");
    }
    //buttonFrame x3
    for (int i = 0; i < 3; ++i)
    {
        No::Entity buttonFrameEntity = registry.GenerateEntity();
        registry.AddComponent<StatusSpriteTag>(buttonFrameEntity);
        auto* ts = registry.AddComponent<No::Transform2DComponent>(buttonFrameEntity);
        ts->scale = { 320,144 };
        auto* sprite = registry.AddComponent<No::SpriteComponent>(buttonFrameEntity);
        sprite->name = "buttonFrame";
        sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Sprite/buttonFrame.png");
    }
}

void GameScene::CreateSprite(No::Registry& registry, const std::string& fileName, const std::string& configName)
{
    No::Entity entity = registry.GenerateEntity();

    auto* t2d = registry.AddComponent<No::Transform2DComponent>(entity);

    auto* sprite = registry.AddComponent<No::SpriteComponent>(entity);
    sprite->name = configName; // JSON のキーと一致させる
    //sprite->name = fileName.substr(0, fileName.rfind("."));
    std::string filePath = "resources/game/td_2304/Texture/" + fileName;
    sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture(filePath);
    // JSON 設定を適用 
    SpriteConfigManager::Get().ApplyToSprite(*sprite, *t2d);
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

NoEngine::Vector3 GameScene::GenerateRandomPointInCircle(float minRadius, float maxRadius)
{
    // 0〜1の乱数を生成して√で均等分布に
    float raw = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    float r = std::sqrt(raw) * (maxRadius - minRadius) + minRadius;

    // 0〜2πの角度をランダムに生成
    float angle = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * 3.14159265f;

    float x = r * std::cos(angle);
    float y = r * std::sin(angle);

    return Vector3{ x, y, 0.0f }; // Zは0で平面上に配置
}

//NoEngine::Vector3 GameScene::GenerateRandomPointInCircle(float radius)
//{
//
//   int randomAngle =  rand() % 628;
//   float angle = randomAngle * 0.01f;
//
//   // 0〜1の乱数を生成して√で均等分布に 
//  float raw = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
//  float r = radius * std::sqrt(raw);
//
//    float x = r * std::cos(angle);
//    float y = r * std::sin(angle);
//
//    return Vector3{ x, y, -0.5f }; // Zは0で平面上に配置
//}

void GameScene::SoundLoad()
{
    No::SoundLoad(L"resources/game/td_2304//Audio/BGM/batBGM.mp3", "batBGM");
    No::SoundLoad(L"resources/game/td_2304//Audio/BGM/secondBGM.mp3", "secondBGM");
    No::SoundLoad(L"resources/game/td_2304//Audio/BGM/titleBGM.mp3", "titleBGM");

    No::SoundLoad(L"resources/game/td_2304//Audio/SE/ballPong.mp3", "ballPong");
    No::SoundLoad(L"resources/game/td_2304//Audio/SE/ballPong2.mp3", "ballPong2");

    No::SoundLoad(L"resources/game/td_2304//Audio/SE/chargeEnter.mp3", "chargeEnter");

    No::SoundLoad(L"resources/game/td_2304//Audio/SE/batDie.mp3", "batDie");

    {
        No::SoundLoad(L"resources/game/td_2304//Audio/Voice/batGirlLaugh.mp3", "batGirlLaugh");
        No::SoundLoad(L"resources/game/td_2304//Audio/Voice/batGirl_omedetou.mp3", "batGirl_omedetou");
        No::SoundLoad(L"resources/game/td_2304//Audio/Voice/batGirl_yaho.mp3", "batGirl_yaho");
        No::SoundLoad(L"resources/game/td_2304//Audio/Voice/batGirl_huwa.mp3", "batGirl_huwa");
    }


    No::SoundPlay("titleBGM", 0.125f, true);
}
