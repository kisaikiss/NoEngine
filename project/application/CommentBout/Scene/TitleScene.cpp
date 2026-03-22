#include "TitleScene.h"
#include "application/CommentBout/GameTag.h"
#include "application/CommentBout/Component/TitleMenuStateComponent.h"
#include "application/CommentBout/Component/TitleMenuConfigComponent.h"
#include "application/CommentBout/Component/OptionStateComponent.h"
#include "application/CommentBout/Component/OptionMenuConfigComponent.h"
#include "application/CommentBout/System/TitleSystem.h"
#include "application/CommentBout/System/TitleViewSystem.h"
#include "application/CommentBout/System/OptionSystem.h"
#include "application/CommentBout/System/OptionViewSystem.h"
#include "application/CommentBout/Spawner/TitleMenuSpawner.h"
#include "application/CommentBout/Spawner/OptionMenuSpawner.h"

namespace {
	No::Entity cameraE;
}

void TitleScene::Setup()
{
	AddSystem(std::make_unique<TitleSystem>());
	AddSystem(std::make_unique<OptionSystem>());
	AddSystem(std::make_unique<TitleViewSystem>());
	AddSystem(std::make_unique<OptionViewSystem>());
	AddSystem(std::make_unique<No::EditSystem>());
	AddSystem(std::make_unique<No::DebugCameraSystem>());
	AddSystem(std::make_unique<No::CameraSystem>());

	No::Registry& registry = *GetRegistry();
	auto light = registry.GenerateEntity();
	auto* dir = registry.AddComponent<No::DirectionalLightComponent>(light);
	dir->color = { 1.f, 1.f, 1.f, 1.f };
	dir->direction = { 0.f, -1.f, 0.f };
	dir->intensity = 1.f;

	auto camera = registry.GenerateEntity();
	registry.AddComponent<No::ActiveCameraTag>(camera);
	registry.AddComponent<No::CameraComponent>(camera);
	registry.AddComponent<No::DebugCameraComponent>(camera);
	registry.AddComponent<No::EditTag>(camera)->name = "camera";
	auto* cameraTransform = registry.AddComponent<No::TransformComponent>(camera);
	cameraTransform->translate.z = -5.f;

	cameraE = registry.GenerateEntity();
	registry.AddComponent<No::CameraComponent>(cameraE);
	auto* cameraTransform2 = registry.AddComponent<No::TransformComponent>(cameraE);
	cameraTransform2->translate.z = -5.f;
	auto titleStateEntity = registry.GenerateEntity();
	registry.AddComponent<CBTitleStateTag>(titleStateEntity);
	auto* titleState = registry.AddComponent<TitleMenuStateComponent>(titleStateEntity);
	titleState->selectedIndex = 0;
	titleState->itemCount = 3;
	titleState->isConfirmAnimating = false;
	titleState->confirmIndex = -1;
	titleState->confirmAnimTime = 0.0f;
	titleState->requestedAction = TitleMenuStateComponent::None;
	titleState->logoMotionTime = 0.0f;
	registry.AddComponent<No::EditTag>(titleStateEntity)->name = "TitleMenuState";

	auto titleConfigEntity = registry.GenerateEntity();
	registry.AddComponent<CBTitleConfigTag>(titleConfigEntity);
	registry.AddComponent<TitleMenuConfigComponent>(titleConfigEntity);
	registry.AddComponent<No::EditTag>(titleConfigEntity)->name = "TitleMenuConfig";

	auto optionStateEntity = registry.GenerateEntity();
	registry.AddComponent<CBOptionStateTag>(optionStateEntity);
	auto* optionState = registry.AddComponent<OptionStateComponent>(optionStateEntity);
	optionState->isOpen = false;
	optionState->phase = OptionStateComponent::Closed;
	optionState->phaseDuration = 1.0f;
	optionState->selectedIndex = 0;
	optionState->isEditing = false;
	optionState->isConfirmAnimating = false;
	optionState->confirmIndex = -1;
	optionState->confirmAnimTime = 0.0f;
	optionState->requestedAction = OptionStateComponent::None;
	registry.AddComponent<No::EditTag>(optionStateEntity)->name = "TitleOptionState";

	auto optionConfigEntity = registry.GenerateEntity();
	registry.AddComponent<CBOptionConfigTag>(optionConfigEntity);
	registry.AddComponent<OptionMenuConfigComponent>(optionConfigEntity);
	registry.AddComponent<No::EditTag>(optionConfigEntity)->name = "TitleOptionMenuConfig";

	const auto whiteTexture = NoEngine::TextureManager::LoadCovertTexture("resources/engine/white1x1.png");
	TitleMenuSpawner::Create(registry, whiteTexture);
	OptionMenuSpawner::Create(registry, whiteTexture);
}

void TitleScene::NotSystemUpdate()
{
#ifdef USE_IMGUI
	No::Registry& registry = *GetRegistry();
	OptionStateComponent* optionState = nullptr;
	auto optionStateView = registry.View<CBOptionStateTag, OptionStateComponent>();
	for (auto entity : optionStateView) {
		optionState = registry.GetComponent<OptionStateComponent>(entity);
		if (optionState) break;
	}

	ImGui::Begin("ChangeScene");
	if (ImGui::Button("OpenOption")) {
		CommentBout::OpenOptionMenuEvent event;
		event.owner = CommentBout::OptionMenuOwner::Title;
		GetRegistry()->EmitEvent(event);
	}
	if ((!optionState || !optionState->isOpen) && ImGui::Button("SceneChange")) {
		No::SceneChangeEvent event;
		event.nextScene = "GameScene";
		GetRegistry()->EmitEvent(event);
	}
	if (ImGui::Button("CameraChange")) {
		GetRegistry()->AddComponent<No::ActiveCameraTag>(cameraE);
	}
	ImGui::End();
#endif
}