#include "TitleScene.h"
#include"engine/Functions/ECS/Component/SpriteComponent.h"
#include"engine/Functions/ECS/Component/Transform2DComponent.h"
#include "../Component/BackGroundComponent.h"

#include"engine/Math/MathInclude.h"

#include "../System/TitleSystem.h"
#include "../System/BackGroundEffectSystem.h"

using namespace NoEngine;
void TitleScene::Setup()
{
	AddSystem(std::make_unique<TitleSystem>());
	AddSystem(std::make_unique<BackGroundEffectSystem>());

	auto& registry = *GetRegistry();
	{
		auto light = registry.GenerateEntity();
		auto* dir = registry.AddComponent<No::DirectionalLightComponent>(light);
		dir->color = { 1.f,1.f,1.f,1.f };
		dir->direction = { 0.f,-1.f,0.f };
		dir->intensity = 1.f;
	}

	InitTitle(registry);

	//背景初期化
	{
		No::Entity backGroundEntity = registry.GenerateEntity();
		auto* transform = registry.AddComponent<No::TransformComponent>(backGroundEntity);
		transform->translate.z = 5;
		transform->scale = { 15,15,1 };

		auto* back = registry.AddComponent<BackGroundComponent>(backGroundEntity);
		back->seed = 1;
		back->bgColor = No::Color(0x11278FFF);
	}
	constexpr No::Vector3 kStartCameraPosition = No::Vector3{ 0.0f, 0.0f, -10.0f };
	//カメラ初期化
	camera_ = std::make_unique<NoEngine::Camera>();
	cameraTransform_.translate = kStartCameraPosition;
	camera_->SetTransform(cameraTransform_);
	SetCamera(camera_.get());

	//BGMの読み込み
	No::SoundLoad(L"resources/game/td_2304/Audio/BGM/titleBGM.mp3", "titleBGM");
	No::SoundCompleteStop("chefBGM");
	No::SoundCompleteStop("rapMusic");
	No::SoundCompleteStop("secondBGM");
	No::SoundCompleteStop("batBGM");
	No::SoundCompleteStop("titleBGM");

	No::SoundPlay("titleBGM", 0.125f, true);
	No::SoundLoad(L"resources/game/td_2304/Audio/SE/select.mp3", "select");
}

void TitleScene::NotSystemUpdate()
{
#ifdef USE_IMGUI
	ImGui::Begin("camera");
	ImGui::DragFloat3("pos", &cameraTransform_.translate.x, 0.1f);
	ImGui::End();
	camera_->SetTransform(cameraTransform_);
#endif // USE_IMGUI
}

void TitleScene::InitTitle(No::Registry& registry)
{	//BGMの再生
	//No::SoundStop("");


	// タイトル文字
	{
		auto titleFont = registry.GenerateEntity();
		auto* title = registry.AddComponent<No::SpriteComponent>(titleFont);
		title->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Sprite/title.png");
		title->name = "Title";

		auto* titleTransform = registry.AddComponent<No::Transform2DComponent>(titleFont);
		titleTransform->scale = { 904.0f, 460.0f };
		titleTransform->translate = { 640.0f,252.0f };
	}

	// スタートボタン
	{
		auto startFont = registry.GenerateEntity();
		auto* start = registry.AddComponent<No::SpriteComponent>(startFont);
		start->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Sprite/startButton.png");
		start->name = "StartButton";

		auto* startTransform = registry.AddComponent<No::Transform2DComponent>(startFont);
		startTransform->scale = { 410.0f, 163.0f };
		startTransform->translate = { 640.0f,565.0f };
	}

	// A button
	{
		auto aButton = registry.GenerateEntity();
		auto* aButtonSprite = registry.AddComponent<No::SpriteComponent>(aButton);
		aButtonSprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_2304/Sprite/aButton.png");
		aButtonSprite->name = "AButton";

		auto* aButtonTransform = registry.AddComponent<No::Transform2DComponent>(aButton);
		aButtonTransform->scale = { 114.0f, 119.0f };
		aButtonTransform->translate = { 639.0f,655.0f };
	}
}
