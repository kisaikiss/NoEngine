#include "ResultScene.h"
#include"engine/Functions/ECS/Component/SpriteComponent.h"
#include"engine/Functions/ECS/Component/Transform2DComponent.h"
#include "../Component/BackGroundComponent.h"
#include "../Component/PlayerstatusComponent.h"
#include "../Component/ScoreDigitComponent.h"

#include "../System/ScoreRankingSystem.h"
#include"engine/Math/MathInclude.h"

#include "../System/BackGroundEffectSystem.h"

#include "../System/Human/ResultChefControlSystem.h"
#include"../System/Human/ResultPlayerGirlControlSystem.h"
#include "../tag.h"

using namespace NoEngine;

void ResultScene::Setup()
{
    AddSystem(std::make_unique<No::AnimationSystem>());
    AddSystem(std::make_unique<BackGroundEffectSystem>());
    AddSystem(std::make_unique<ScoreRankingSystem>());
    AddSystem(std::make_unique<ResultPlayerGirlControlSystem>());
    AddSystem(std::make_unique<ResultChefControlSystem>());

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
        back->seed = 12;
        back->bgColor = Color(0x230692FF);
        back->ringColor = Color(0x6A4AE4FF);
    }

    InitPlayerScore();
    InitRankingSprite();
    InitPlayerGirl();
    InitChef();

    constexpr Vector3 kStartCameraPosition = Vector3{ 0.0f, 0.0f, -10.0f };
    //カメラ初期化
    camera_ = std::make_unique<NoEngine::Camera>();
    cameraTransform_.translate = kStartCameraPosition;
    camera_->SetTransform(cameraTransform_);
    SetCamera(camera_.get());

    No::SoundLoad(L"resources/game/td_2304/Audio/BGM/rapMusic.mp3", "rapMusic");
    No::SoundCompleteStop("secondBGM");
    No::SoundCompleteStop("batBGM");
    No::SoundCompleteStop("titleBGM");
    No::SoundCompleteStop("chefBGM");
    No::SoundPlay("rapMusic", 0.25f, true);

}

void ResultScene::NotSystemUpdate()
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
        //SE再生 
        No::SoundEffectPlay("select", 0.5f);
		GetRegistry()->EmitEvent(NoEngine::Event::SceneChangeEvent("TitleScene"));
	}
}

void ResultScene::InitPlayerScore()
{
	auto& registry = *GetRegistry();
	No::Entity playerStatusEntity = registry.GenerateEntity();
	registry.AddComponent<PlayerStatusComponent>(playerStatusEntity);

}

void ResultScene::InitRankingSprite()
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

void ResultScene::InitPlayerGirl()
{
    auto& registry = *GetRegistry();
    No::Entity playerGirlEntity = registry.GenerateEntity();
    registry.AddComponent<PlayerGirlTag>(playerGirlEntity);

    auto* transform = registry.AddComponent<No::TransformComponent>(playerGirlEntity);
    auto* model = registry.AddComponent<No::MeshComponent>(playerGirlEntity);
    auto* animationComp = registry.AddComponent<No::AnimatorComponent>(playerGirlEntity);
    NoEngine::ModelLoader::LoadModel("playerGirl", "resources/game/td_2304/Model/playerGirl/playerGirl.gltf", model, animationComp);

    transform->translate = { -1.00f,-1.85f,3.0f };
    transform->scale = { 0.15f,0.15f,0.15f };
    transform->rotation.FromAxisAngle(NoEngine::Vector3::UP, 3.14f - 0.5f);

    auto m = registry.AddComponent<No::MaterialComponent>(playerGirlEntity);
    m->materials = NoEngine::ModelLoader::GetMaterial("playerGirl");

    m->psoName = L"Renderer : ToonSkinned PSO";
    m->enableSkinning = true;
    m->drawOutline = true;
    m->psoId = NoEngine::Render::GetPSOID(m->psoName);
    m->rootSigId = NoEngine::Render::GetRootSignatureID(m->psoName);
}

void ResultScene::InitChef()
{
    auto& registry = *GetRegistry();
    No::Entity chefEntity = registry.GenerateEntity();
    registry.AddComponent<ChefTag>(chefEntity);

    auto* transform = registry.AddComponent<No::TransformComponent>(chefEntity);
    auto* model = registry.AddComponent<No::MeshComponent>(chefEntity);
    auto* animationComp = registry.AddComponent<No::AnimatorComponent>(chefEntity);
    NoEngine::ModelLoader::LoadModel("chef", "resources/game/td_2304/Model/man/man.gltf", model, animationComp);

    transform->translate = { 1.35f,-2.15f,4.05f };
    transform->scale = { 0.15f,0.15f,0.15f };
    transform->rotation.FromAxisAngle(NoEngine::Vector3::UP, 3.14f);

    auto m = registry.AddComponent<No::MaterialComponent>(chefEntity);
    m->materials = NoEngine::ModelLoader::GetMaterial("chef");

    m->psoName = L"Renderer : ToonSkinned PSO";
    m->enableSkinning = true;
    m->drawOutline = true;
    m->psoId = NoEngine::Render::GetPSOID(m->psoName);
    m->rootSigId = NoEngine::Render::GetRootSignatureID(m->psoName);
}
