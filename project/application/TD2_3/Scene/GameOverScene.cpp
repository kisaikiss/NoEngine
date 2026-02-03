#include "GameOverScene.h"
#include"engine/Functions/ECS/Component/SpriteComponent.h"
#include"engine/Functions/ECS/Component/Transform2DComponent.h"
#include "../Component/BackGroundComponent.h"

#include"engine/Math/MathInclude.h"

#include "../System/GameOverSystem.h"
#include "../System/BackGroundEffectSystem.h"

using namespace NoEngine;
void GameOverScene::Setup()
{
	AddSystem(std::make_unique<GameOverSystem>());
	AddSystem(std::make_unique<BackGroundEffectSystem>());

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

		registry.AddComponent<BackGroundComponent>(backGroundEntity);
	}

	constexpr Vector3 kStartCameraPosition = Vector3{ 0.0f, 0.0f, -10.0f };
	//カメラ初期化
	camera_ = std::make_unique<NoEngine::Camera>();
	cameraTransform_.translate = kStartCameraPosition;
	camera_->SetTransform(cameraTransform_);
	SetCamera(camera_.get());

}

void GameOverScene::NotSystemUpdate()
{
	if (NoEngine::Input::Keyboard::IsTrigger(VK_RETURN))
	{
		GetRegistry()->EmitEvent(NoEngine::Event::SceneChangeEvent("TitleScene"));
	}
}
