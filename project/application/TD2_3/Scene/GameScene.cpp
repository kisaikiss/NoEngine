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
#include "../Component/PhaseComponent.h"
#include "../Component/ScoreDigitComponent.h"
#include "../Component/TutorialSpriteComponent.h"
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
#include "../System/Enemy/BatEnemy/BatControlSystem.h"
//Human
#include "../System/Human/BatGirlControlSystem.h"
#include "../System/Human/ChefControlSystem.h"
#include "../System/Human/PlayerGirlControlSystem.h"
#include"../System/Human/HumanControlSystem.h"

#include"../System/StatusSpriteControlSystem.h"
#include"../SpriteConfigManager/SpriteConfigManager.h"
#include "../System/Tutorial/TutorialControlSystem.h"
//ヨシダ追加しました。

// score
#include "../System/Sprite/ScoreSpriteControlSystem.h"
// bat
#include "../Component/Enemy/BatComponent.h"
#include "../System/Enemy/BatEnemy/BatGenerateSystem.h"
#include "../System/Enemy/BatEnemy/BatGreenControlSystem.h"
#include "../System/Enemy/EnemyBulletControlSystem.h"

// boss
#include "../System/Enemy/Boss/BossGenerateSystem.h"

#include "../System/Enemy/EnemyPushBackSystem.h"

// effect
#include "../System/Effect/SmokeEffectControlSystem.h"

#include "../System/Enemy/CookingEnemy/RootVegetableGenerateSystem.h"
#include "../System/Enemy/CookingEnemy/WhiteRadishControlSystem.h"
#include "../System/Enemy/CookingEnemy/CarrotControlSystem.h"
#include "../System/Enemy/CookingEnemy/PotControlSystem.h"
#include "../System/Enemy/CookingEnemy/IngredientsControlSystem.h"

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
    AddSystem(std::make_unique<BatControlSystem>());
    AddSystem(std::make_unique<BatGreenControlSystem>());
    AddSystem(std::make_unique<BatGenerateSystem>());
    AddSystem(std::make_unique<WhiteRadishControlSystem>());
    AddSystem(std::make_unique<CarrotControlSystem>());
    AddSystem(std::make_unique<RootVegetableGenerateSystem>());
    AddSystem(std::make_unique<BossGenerateSystem>());
    AddSystem(std::make_unique<BossControlSystem>());
    AddSystem(std::make_unique<PotControlSystem>());
    AddSystem(std::make_unique<EnemyBulletControlSystem>());
    AddSystem(std::make_unique<IngredientsControlSystem>());
    //HumanControlSystem シーン切り替え時の処理用
    AddSystem(std::make_unique<HumanControlSystem>());
    //こうもり少女のシステム
    AddSystem(std::make_unique<BatGirlControlSystem>());
    //シェフシステム
    AddSystem(std::make_unique<ChefControlSystem>());
    //プレイヤー少女システム
    AddSystem(std::make_unique<PlayerGirlControlSystem>());
    //playerステータス管理システム
    AddSystem(std::make_unique<PlayerStatusSystem>());
    AddSystem(std::make_unique<UpgradeSelectionSystem>());
    //Tutorial
    AddSystem(std::make_unique<TutorialControlSystem>());
    //HISprite
    AddSystem(std::make_unique<StatusSpriteControlSystem>());
    //衝突判定用システム
    AddSystem(std::make_unique<CollisionSystem>());
    AddSystem(std::make_unique<EnemyPushBackSystem>());

    // スコア描画のためのコントロールシステム
    AddSystem(std::make_unique<ScoreSpriteControlSystem>());
    // 煙エフェクト
    AddSystem(std::make_unique<SmokeEffectControlSystem>());


    SpriteConfigManager::Get().Load("resources/game/td_2304/Json/tdSpriteConfig.json");

    No::Registry& registry = *GetRegistry();
    InitBackGround(registry);
    InitVaus(registry);
    //InitEnemy(registry);
    //phaseの初期化
    InitPhase(registry);
    InitBat(registry);
    InitRing(registry);
    InitBall(registry);
    InitHumanParent(registry);
    InitBatGirl(registry);
    InitPlayerGirl(registry);
    //シェフの初期化
    InitChef(registry);
    InitPlayerStatus(registry);
    InitLights(registry);
    InitHpGaugeSprite(registry);
    InitLevelGaugeSprite(registry);
    InitChooseSprite(registry);
    InitScore(registry);
    InitTutorialSprite(registry);
    constexpr Vector3 kStartCameraPosition = Vector3{ 0.0f, 0.0f, -28.0f };
    //カメラ初期化
    camera_ = std::make_unique<NoEngine::Camera>();
    cameraTransform_.translate = kStartCameraPosition;
    camera_->SetTransform(cameraTransform_);
    SetCamera(camera_.get());

    ModelLoad();
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

    if (Input::Keyboard::IsTrigger(VK_F1) || Input::Pad::IsTrigger(Input::GamepadButton::Start))
    {
        GetRegistry()->EmitEvent(Event::SceneChangeEvent("TitleScene"));
    }
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
    m->drawOutline = true;

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
        m->drawOutline = true;
    }
}

void GameScene::InitBackGround(No::Registry& registry)
{
    No::Entity backGroundEntity = registry.GenerateEntity();
    auto* transform = registry.AddComponent<No::TransformComponent>(backGroundEntity);

    transform->translate.z = 5;
    transform->scale = { 30,30,1 };

    auto* back = registry.AddComponent<BackGroundComponent>(backGroundEntity);
    back->seed = 123;
	back->useRing = 1;
}

void GameScene::InitBatGirl(No::Registry& registry)
{
    No::Entity batGirlEntity = registry.GenerateEntity();
    registry.AddComponent<BatGirlTag>(batGirlEntity);

    auto* transform = registry.AddComponent<No::TransformComponent>(batGirlEntity);
    auto* model = registry.AddComponent<No::MeshComponent>(batGirlEntity);
    auto* animationComp = registry.AddComponent<No::AnimatorComponent>(batGirlEntity);
    NoEngine::ModelLoader::LoadModel("batGirl", "resources/game/td_2304/Model/batGirl/batGirl.gltf", model, animationComp);

    transform->translate = { -2.75f,0.0f,0.0f };
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

void GameScene::InitChef(No::Registry& registry)
{


    No::Entity chefEntity = registry.GenerateEntity();
    registry.AddComponent<ChefTag>(chefEntity);

    auto* transform = registry.AddComponent<No::TransformComponent>(chefEntity);
    auto* model = registry.AddComponent<No::MeshComponent>(chefEntity);
    auto* animationComp = registry.AddComponent<No::AnimatorComponent>(chefEntity);
    NoEngine::ModelLoader::LoadModel("chef", "resources/game/td_2304/Model/man/man.gltf", model, animationComp);

    transform->translate = { 3.00f,0.0f,0.0f };

    auto m = registry.AddComponent<No::MaterialComponent>(chefEntity);
    m->materials = NoEngine::ModelLoader::GetMaterial("chef");

    m->psoName = L"Renderer : ToonSkinned PSO";
    m->enableSkinning = true;
    m->drawOutline = true;
    m->psoId = NoEngine::Render::GetPSOID(m->psoName);
    m->rootSigId = NoEngine::Render::GetRootSignatureID(m->psoName);

}

void GameScene::InitHumanParent(No::Registry& registry)
{
    No::Entity humanControlSystem = registry.GenerateEntity();
    auto* transform = registry.AddComponent<No::TransformComponent>(humanControlSystem);
    transform->translate = { 9.5f, -14.55f, -8.5f };
    registry.AddComponent<EnemyHumanTag>(humanControlSystem);
}

void GameScene::InitPlayerStatus(No::Registry& registry)
{
    No::Entity playerStatusEntity = registry.GenerateEntity();
    auto* status = registry.AddComponent<PlayerStatusComponent>(playerStatusEntity);
    status->exp = 0;
    status->hp = 5;
    status->level = 1;
    status->score = 0;
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
    auto hpBar = CreateSprite(registry, "hpBar.png", "PlayerHpBar");
    auto hpGauge = CreateSprite(registry, "hpGaugeColor.png", "HpGauge");
    auto* hpBarTransform = registry.GetComponent<No::Transform2DComponent>(hpBar);

    auto* hpGaugeTransform = registry.GetComponent<No::Transform2DComponent>(hpGauge);
    hpGaugeTransform->translate = hpBarTransform->translate;
    hpGaugeTransform->scale = hpBarTransform->scale;

    auto* hpGaugeSprite = registry.GetComponent<No::SpriteComponent>(hpGauge);
    hpGaugeSprite->useMask = 1;
    hpGaugeSprite->maskTextureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Sprite/hpGageMask.png");

}

void GameScene::InitLevelGaugeSprite(No::Registry& registry)
{
    auto lvSp = CreateSprite(registry, "lv.png", "Level");
    auto lvGauge = CreateSprite(registry, "lv_gauge.png", "LevelGauge");
    auto* lvTransform = registry.GetComponent<No::Transform2DComponent>(lvSp);

    auto* lvGaugeTransform = registry.GetComponent<No::Transform2DComponent>(lvGauge);
    lvGaugeTransform->translate = lvTransform->translate;
    lvGaugeTransform->scale = lvTransform->scale;

    auto* lvGaugeSprite = registry.GetComponent<No::SpriteComponent>(lvGauge);
    lvGaugeSprite->useMask = 1;
    lvGaugeSprite->maskTextureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Sprite/lv_gauge_mask.png");

    auto nums = registry.GenerateEntity();
    auto* sprite = registry.AddComponent<No::SpriteComponent>(nums);
    auto* transform = registry.AddComponent<No::Transform2DComponent>(nums);
    transform->scale = { 64.f,64.f };
    sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Sprite/levelNumbers.png");
    sprite->uv.width = 0.1f;
    sprite->name = "LevelFont";
    sprite->layer = 2;
}

void GameScene::InitTutorialSprite(No::Registry& registry)
{

    //stickL
    {
        No::Entity stickLEntity = registry.GenerateEntity();
        registry.AddComponent<TutorialSpriteTag>(stickLEntity);
        auto* ts = registry.AddComponent<No::Transform2DComponent>(stickLEntity);
        registry.AddComponent<TutorialSpriteComponent>(stickLEntity);
    
        ts->scale = { 72, 80 };
        ts->translate = { -1000.f,360.0f };

        auto* sprite = registry.AddComponent<No::SpriteComponent>(stickLEntity);
        sprite->name = "stickL";

        sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Sprite/stickL.png");
    }

    //cursorMove
    {
        No::Entity stickLEntity = registry.GenerateEntity();
        registry.AddComponent<TutorialSpriteTag>(stickLEntity);
        auto* ts = registry.AddComponent<No::Transform2DComponent>(stickLEntity);
        registry.AddComponent<TutorialSpriteComponent>(stickLEntity);
        ts->scale = { 58,80 };
        ts->translate = { -1000.f,360.0f };
        auto* sprite = registry.AddComponent<No::SpriteComponent>(stickLEntity);
        sprite->name = "cursorMove";
        sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Sprite/cursorMove.png");
    }

    //GameButtonA
    {
        No::Entity stickLEntity = registry.GenerateEntity();
        registry.AddComponent<TutorialSpriteTag>(stickLEntity);
        auto* ts = registry.AddComponent<No::Transform2DComponent>(stickLEntity);
        registry.AddComponent<TutorialSpriteComponent>(stickLEntity);
        ts->scale = { 61, 76 };
        ts->translate = { -1000.f,360.0f };
        auto* sprite = registry.AddComponent<No::SpriteComponent>(stickLEntity);
        sprite->name = "gameAButton2";
        sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Sprite/gameAButton2.png");
    }


    //CursorPush
    {
        No::Entity stickLEntity = registry.GenerateEntity();
        registry.AddComponent<TutorialSpriteTag>(stickLEntity);
        auto* ts = registry.AddComponent<No::Transform2DComponent>(stickLEntity);
        registry.AddComponent<TutorialSpriteComponent>(stickLEntity);
        ts->scale = { 62, 72 };
        ts->translate = { -1000.f,360.0f };
        auto* sprite = registry.AddComponent<No::SpriteComponent>(stickLEntity);
        sprite->name = "cursorPush";
        sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Sprite/cursorPush.png");
    }


    //PaddleRound
    {
        No::Entity stickLEntity = registry.GenerateEntity();
        registry.AddComponent<TutorialSpriteTag>(stickLEntity);
        auto* ts = registry.AddComponent<No::Transform2DComponent>(stickLEntity);
        registry.AddComponent<TutorialSpriteComponent>(stickLEntity);
        ts->scale = { 424,74 };
        ts->translate = { -1000.f,360.0f };
        auto* sprite = registry.AddComponent<No::SpriteComponent>(stickLEntity);
        sprite->name = "paddleRound";
        sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Sprite/paddle.png");
    }

    //throwBall
    {
        No::Entity stickLEntity = registry.GenerateEntity();
        registry.AddComponent<TutorialSpriteTag>(stickLEntity);
        auto* ts = registry.AddComponent<No::Transform2DComponent>(stickLEntity);
        registry.AddComponent<TutorialSpriteComponent>(stickLEntity);
        ts->scale = { 402,82 };
        ts->translate = { -1000.f,360.0f };
        auto* sprite = registry.AddComponent<No::SpriteComponent>(stickLEntity);
        sprite->name = "throwBall";
        sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Sprite/throwBall.png");
    }
}

void GameScene::InitScore(No::Registry& registry) {
    const uint32_t kDigits = 6;


    for (uint32_t i = 0; i < kDigits; i++) {
        auto nums = registry.GenerateEntity();
        auto* sprite = registry.AddComponent<No::SpriteComponent>(nums);
        auto* transform = registry.AddComponent<No::Transform2DComponent>(nums);
        transform->scale = { 64.f,64.f };
        registry.AddComponent<ScoreDigitComponent>(nums);
        sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Sprite/numbers.png");
        sprite->uv.width = 0.1f;
    }

    auto score = registry.GenerateEntity();
    auto* sprite = registry.AddComponent<No::SpriteComponent>(score);
    auto* transform = registry.AddComponent<No::Transform2DComponent>(score);

    sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Sprite/score.png");
    transform->scale = { 236.f,52.f };
    transform->translate = { 900.f,50.f };
}

void GameScene::InitBat(No::Registry& registry) {
    for (int i = 0; i < 5; ++i) {
        No::Entity entity = registry.GenerateEntity();
        registry.AddComponent<EnemyTag>(entity);
        registry.AddComponent<BatTag>(entity);
        registry.AddComponent<DeathFlag>(entity);

        auto* enemy = registry.AddComponent<BatComponent>(entity);

        auto* collider = registry.AddComponent<SphereColliderComponent>(entity);
        collider->colliderType = ColliderMask::kEnemy;
        collider->collideMask = ColliderMask::kBall;

        auto* transform = registry.AddComponent<No::TransformComponent>(entity);
        transform->rotation.FromAxisAngle(Vector3::UP, 3.14f);
        transform->translate = GenerateRandomPointInCircle(2.0f, 3.0f);
        transform->scale = 0.f;

        enemy->defaultTranslate = transform->translate;

        auto* model = registry.AddComponent<No::MeshComponent>(entity);
        auto* animationComp = registry.AddComponent<No::AnimatorComponent>(entity);
        NoEngine::ModelLoader::LoadModel(enemyResources_.modelName, enemyResources_.modelPath, model, animationComp);

        auto m = registry.AddComponent<No::MaterialComponent>(entity);
        m->materials = NoEngine::ModelLoader::GetMaterial("bat");
        // m->materials[0].textureHandle = NoEngine::TextureManager::LoadCovertTexture(enemyResources_.texturePath);
        m->psoName = L"Renderer : Default PSO";
        m->psoId = NoEngine::Render::GetPSOID(m->psoName);
        m->rootSigId = NoEngine::Render::GetRootSignatureID(m->psoName);
        m->drawOutline = true;
    }
}

void GameScene::InitPhase(No::Registry& registry)
{
    No::Entity entity = registry.GenerateEntity();
    auto* phase = registry.AddComponent<PhaseComponent>(entity);
    phase->phase = Phase::ONE;
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

No::Entity  GameScene::CreateSprite(No::Registry& registry, const std::string& fileName, const std::string& configName)
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
    return entity;
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


    No::SoundLoad(L"resources/game/td_2304//Audio/SE/ballPong.mp3", "ballPong");
    No::SoundLoad(L"resources/game/td_2304//Audio/SE/ballPong2.mp3", "ballPong2");

    No::SoundLoad(L"resources/game/td_2304//Audio/SE/chargeEnter.mp3", "chargeEnter");

    No::SoundLoad(L"resources/game/td_2304//Audio/SE/batDie.mp3", "batDie");
    
    No::SoundLoad(L"resources/game/td_2304//Audio/SE/levelUp.mp3", "levelUp");

    No::SoundLoad(L"resources/game/td_2304//Audio/SE/upgrade.mp3", "upgrade");


    No::SoundCompleteStop("titleBGM");
    No::SoundCompleteStop("batBGM");
    No::SoundPlay("batBGM", 0.25f, true);
}

void GameScene::ModelLoad() {
    NoEngine::ModelLoader::LoadModel("batBoss", "resources/game/td_2304/Model/batBoss/batBoss.gltf");
    NoEngine::ModelLoader::LoadModel("potBoss", "resources/game/td_2304/Model/pot/pot.gltf");
}
