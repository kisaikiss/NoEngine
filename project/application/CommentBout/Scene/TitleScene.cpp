#include "TitleScene.h"
#include "application/CommentBout/GameTag.h"
#include "application/CommentBout/Component/GameResourceComponent.h"
#include "application/CommentBout/Component/OutGame/TitleMenuStateComponent.h"
#include "application/CommentBout/Component/OutGame/TitleMenuConfigComponent.h"
#include "application/CommentBout/Component/OutGame/OptionStateComponent.h"
#include "application/CommentBout/Component/OutGame/OptionMenuConfigComponent.h"
#include "application/CommentBout/System/InputHelperSystem.h"
#include "application/CommentBout/System/OutGame/TitleSystem.h"
#include "application/CommentBout/System/OutGame/TitleViewSystem.h"
#include "application/CommentBout/System/OutGame/OptionSystem.h"
#include "application/CommentBout/System/OutGame/OptionViewSystem.h"
#include "application/CommentBout/Spawner/TitleMenuSpawner.h"
#include "application/CommentBout/Spawner/OptionMenuSpawner.h"

namespace {
	No::Entity cameraE;
}

void TitleScene::Setup()
{
	AddSystem(std::make_unique<InputHelperSystem>());
	AddSystem(std::make_unique<TitleSystem>());
	AddSystem(std::make_unique<OptionSystem>());
	AddSystem(std::make_unique<TitleViewSystem>());
	AddSystem(std::make_unique<OptionViewSystem>());
	AddSystem(std::make_unique<No::EditSystem>());
	AddSystem(std::make_unique<No::DebugCameraSystem>());
	AddSystem(std::make_unique<No::CameraSystem>());

	No::Registry& registry = *GetRegistry();

	auto gameResourceEntity = registry.GenerateEntity();
	registry.AddComponent<CBGameResourceTag>(gameResourceEntity);
	auto* gameResource = registry.AddComponent<GameResourceComponent>(gameResourceEntity);
	InitializeCommentBoutGameResources(*gameResource);

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

	TitleMenuSpawner::Create(registry, *gameResource);
	OptionMenuSpawner::Create(registry, *gameResource);
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
	if (ImGui::Button("SceneChange")) {
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