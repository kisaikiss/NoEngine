#include "GameOverScene.h"
#include"engine/Functions/ECS/Component/SpriteComponent.h"
#include"engine/Functions/ECS/Component/Transform2DComponent.h"
#include "../Component/BackGroundComponent.h"
#include "../Component/PlayerstatusComponent.h"
#include "../Component/ScoreDigitComponent.h"

#include "../System/ScoreRankingSystem.h"
#include"engine/Math/MathInclude.h"

#include "../System/BackGroundEffectSystem.h"
#include"../System/Human/HumanControlSystem.h"
#include"../System/Human/BatGirlControlSystem.h"
#include"../System/Human/ChefControlSystem.h"
#include"../System/Human/GameOverPlayerGirlControlSystem.h"
#include "../System/Human/GameOverBatControlSystem.h"
#include "../tag.h"

using namespace NoEngine;
void GameOverScene::Setup()
{
    //アニメーションシステム
    AddSystem(std::make_unique<No::AnimationSystem>());
    AddSystem(std::make_unique<BackGroundEffectSystem>());
    AddSystem(std::make_unique<ScoreRankingSystem>());

    AddSystem(std::make_unique<GameOverPlayerGirlControlSystem>());
    //AddSystem(std::make_unique<ChefControlSystem>());
    AddSystem(std::make_unique<GameOverBatControlSystem>());

    InitPlayerGirl();
    InitBatGirl();
    //InitChef();
    //InitHuman();

    auto& registry = *GetRegistry();
    {
        auto light = registry.GenerateEntity();
        auto* dir = registry.AddComponent<No::DirectionalLightComponent>(light);
        dir->color = { 1.f,1.f,1.f,1.f };
        dir->direction = { 0.f,-1.f,0.f };
        dir->intensity = 1.f;
    }

    //背景初期化
    {
        No::Entity backGroundEntity = registry.GenerateEntity();
        auto* transform = registry.AddComponent<No::TransformComponent>(backGroundEntity);
        transform->translate.z = 5;
        transform->scale = { 15,15,1 };

        auto* back = registry.AddComponent<BackGroundComponent>(backGroundEntity);
        back->seed = 34;
        back->bgColor = Color(0x8E1313FF);
        back->ringColor = Color(0xB92F2FFF);
    }

    InitPlayerScore();
    InitRankingSprite();

    constexpr Vector3 kStartCameraPosition = Vector3{ 0.0f, 0.0f, -28.0f };
    //カメラ初期化
    camera_ = std::make_unique<NoEngine::Camera>();
    cameraTransform_.translate = kStartCameraPosition;
    camera_->SetTransform(cameraTransform_);
    SetCamera(camera_.get());

}

void GameOverScene::NotSystemUpdate()
{
#ifdef USE_IMGUI
    ImGui::Begin("camera");
    ImGui::DragFloat3("pos", &cameraTransform_.translate.x, 0.1f);
    ImGui::End();
    camera_->SetTransform(cameraTransform_);
#endif // USE_IMGUI
    if ((No::Keyboard::IsTrigger(VK_RETURN) ||
        No::Pad::IsTrigger(No::GamepadButton::A)) && !isChangeScene_)
    {
        GetRegistry()->EmitEvent(NoEngine::Event::SceneChangeEvent("TitleScene"));
    }
}

void GameOverScene::InitPlayerScore()
{
    auto& registry = *GetRegistry();
    No::Entity playerStatusEntity = registry.GenerateEntity();
    registry.AddComponent<PlayerStatusComponent>(playerStatusEntity);

}

void GameOverScene::InitRankingSprite()
{
    auto& registry = *GetRegistry();
    No::Entity rankSprite = registry.GenerateEntity();
    auto* sprite = registry.AddComponent<No::SpriteComponent>(rankSprite);
    sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Sprite/ranking.png");
    sprite->name = "Ranking";

    auto* transform = registry.AddComponent<No::Transform2DComponent>(rankSprite);
    transform->scale = { 664.0f,176.0f };
    transform->translate = { 640.0f,200.0f };
}

void GameOverScene::InitHuman()
{
    auto& registry = *GetRegistry();
    No::Entity humanControlSystem = registry.GenerateEntity();
    auto* transform = registry.AddComponent<No::TransformComponent>(humanControlSystem);
    transform->translate = { 9.5f, -14.55f, -8.5f };
    registry.AddComponent<EnemyHumanTag>(humanControlSystem);
}

void GameOverScene::InitPlayerGirl()
{
    auto& registry = *GetRegistry();
    No::Entity playerGirlEntity = registry.GenerateEntity();
    registry.AddComponent<PlayerGirlTag>(playerGirlEntity);

    auto* transform = registry.AddComponent<No::TransformComponent>(playerGirlEntity);
    auto* model = registry.AddComponent<No::MeshComponent>(playerGirlEntity);
    auto* animationComp = registry.AddComponent<No::AnimatorComponent>(playerGirlEntity);
    NoEngine::ModelLoader::LoadModel("playerGirl", "resources/game/td_2304/Model/playerGirl/playerGirl.gltf", model, animationComp);

    transform->translate = { -0.60f,-0.85f,3.25f };
    transform->scale = { 0.15f,0.15f,0.15f };
    transform->rotation.FromAxisAngle(NoEngine::Vector3::UP, 3.14f+0.5f);

    auto m = registry.AddComponent<No::MaterialComponent>(playerGirlEntity);
    m->materials = NoEngine::ModelLoader::GetMaterial("playerGirl");

    m->psoName = L"Renderer : ToonSkinned PSO";
    m->enableSkinning = true;
    m->drawOutline = true;
    m->psoId = NoEngine::Render::GetPSOID(m->psoName);
    m->rootSigId = NoEngine::Render::GetRootSignatureID(m->psoName);
}

void GameOverScene::InitChef()
{
    auto& registry = *GetRegistry();
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

void GameOverScene::InitBatGirl()
{
    auto& registry = *GetRegistry();
    No::Entity batGirlEntity = registry.GenerateEntity();
    registry.AddComponent<BatGirlTag>(batGirlEntity);

    auto* transform = registry.AddComponent<No::TransformComponent>(batGirlEntity);
    auto* model = registry.AddComponent<No::MeshComponent>(batGirlEntity);
    auto* animationComp = registry.AddComponent<No::AnimatorComponent>(batGirlEntity);
    NoEngine::ModelLoader::LoadModel("batGirl", "resources/game/td_2304/Model/batGirl/batGirl.gltf", model, animationComp);

    transform->translate = { 0.95f,2.0f,2.75f };
    transform->scale = { 0.15f,0.15f,0.15f };

    auto m = registry.AddComponent<No::MaterialComponent>(batGirlEntity);
    m->materials = NoEngine::ModelLoader::GetMaterial("batGirl");

    m->psoName = L"Renderer : ToonSkinned PSO";
    m->enableSkinning = true;
    m->drawOutline = true;
    m->psoId = NoEngine::Render::GetPSOID(m->psoName);
    m->rootSigId = NoEngine::Render::GetRootSignatureID(m->psoName);
}
