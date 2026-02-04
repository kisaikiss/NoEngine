#include "ResultScene.h"
#include"engine/Functions/ECS/Component/SpriteComponent.h"
#include"engine/Functions/ECS/Component/Transform2DComponent.h"
#include "../Component/BackGroundComponent.h"
#include "../Component/PlayerstatusComponent.h"
#include "../Component/ScoreDigitComponent.h"

#include "../System/ScoreRankingSystem.h"
#include"engine/Math/MathInclude.h"

#include "../System/BackGroundEffectSystem.h"

using namespace NoEngine;

void ResultScene::Setup()
{
    AddSystem(std::make_unique<BackGroundEffectSystem>());
    AddSystem(std::make_unique<ScoreRankingSystem>());

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

    constexpr Vector3 kStartCameraPosition = Vector3{ 0.0f, 0.0f, -10.0f };
    //カメラ初期化
    camera_ = std::make_unique<NoEngine::Camera>();
    cameraTransform_.translate = kStartCameraPosition;
    camera_->SetTransform(cameraTransform_);
    SetCamera(camera_.get());
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