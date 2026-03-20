#include "TitleScene.h"
#include "application/CommentBout/GameTag.h"
#include "application/CommentBout/Component/OptionStateComponent.h"
#include "application/CommentBout/Component/OptionMenuConfigComponent.h"
#include "application/CommentBout/Component/OptionMenuViewComponent.h"
#include "application/CommentBout/System/OptionSystem.h"
#include "application/CommentBout/System/OptionViewSystem.h"
#include "application/CommentBout/Event/OptionMenuEvent.h"

namespace {
	No::Entity cameraE;
}

void TitleScene::Setup() {
	AddSystem(std::make_unique<No::AnimationSystem>());
	AddSystem(std::make_unique<OptionSystem>());
	AddSystem(std::make_unique<OptionViewSystem>());
	AddSystem(std::make_unique<No::EditSystem>());
	AddSystem(std::make_unique<No::DebugCameraSystem>());
	AddSystem(std::make_unique<No::CameraSystem>());

	No::Registry& registry = *GetRegistry();
	No::Entity entity = registry.GenerateEntity();

	auto* model = registry.AddComponent<No::MeshComponent>(entity);
	auto* t = registry.AddComponent<No::TransformComponent>(entity);
	auto* imguiName = registry.AddComponent<No::EditTag>(entity);
	imguiName->name = "girl";
	t->rotation.FromAxisAngle(No::Vector3(0.f, 1.f, 0.f), PI);
	t->translate = { 0.f,-1.5f,4.f };
	auto* m = registry.AddComponent<No::MaterialComponent>(entity);
	auto* a = registry.AddComponent<No::AnimatorComponent>(entity);
	No::ModelLoader::LoadModel("magiclash", "resources/engine/Model/test/TD_girl/test7.gltf");
	No::ModelLoader::GetModel("magiclash", model, a);
	m->materials = No::ModelLoader::GetMaterial("magiclash");
	m->drawOutline = true;
	m->enableSkinning = true;

	m->psoName = L"Renderer : DefaultSkinned PSO";
	m->psoId = NoEngine::Render::GetPSOID(m->psoName);
	m->rootSigId = NoEngine::Render::GetRootSignatureID(m->psoName);

	auto* t2d = registry.AddComponent<No::Transform2DComponent>(entity);
	t2d->translate = { 100.f, 200.f };
	auto* sprite = registry.AddComponent<No::SpriteComponent>(entity);
	sprite->layer = 1;

	t2d->scale = { 100.f, 100.f };
	sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/engine/Model/enemy.png");

	auto light = registry.GenerateEntity();
	auto* dir = registry.AddComponent<No::DirectionalLightComponent>(light);
	dir->color = { 1.f,1.f,1.f,1.f };
	dir->direction = { 0.f,-1.f,0.f };
	dir->intensity = 1.f;

	auto* t2d2 = registry.AddComponent<No::Transform2DComponent>(light);
	t2d2->translate = { 100.f, 200.f };
	auto* sprite2 = registry.AddComponent<No::SpriteComponent>(light);
	sprite2->layer = 1;

	t2d2->scale = { 100.f, 100.f };
	sprite2->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/engine/Model/enemy.png");

	auto light2 = registry.GenerateEntity();
	auto* dir2 = registry.AddComponent<No::DirectionalLightComponent>(light2);
	dir2->color = { 1.f,1.f,1.f,1.f };
	dir2->direction = { 0.f,-1.f,0.f };
	dir2->intensity = 1.f;

	auto camera = registry.GenerateEntity();
	registry.AddComponent<No::ActiveCameraTag>(camera);
	registry.AddComponent<No::CameraComponent>(camera);
	registry.AddComponent<No::DebugCameraComponent>(camera);
	auto* cameraEditTag = registry.AddComponent<No::EditTag>(camera);
	cameraEditTag->name = "camera";
	auto* cameraTransform = registry.AddComponent<No::TransformComponent>(camera);
	cameraTransform->translate.z = -5.f;

	cameraE = registry.GenerateEntity();
	registry.AddComponent<No::CameraComponent>(cameraE);
	auto* cameraTransform2 = registry.AddComponent<No::TransformComponent>(cameraE);
	cameraTransform2->translate.z = -5.f;

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
	auto* optionStateTag = registry.AddComponent<No::EditTag>(optionStateEntity);
	optionStateTag->name = "TitleOptionState";

	auto optionConfigEntity = registry.GenerateEntity();
	registry.AddComponent<CBOptionConfigTag>(optionConfigEntity);
	auto* optionConfig = registry.AddComponent<OptionMenuConfigComponent>(optionConfigEntity);
	static_cast<void>(optionConfig);
	auto* optionConfigTag = registry.AddComponent<No::EditTag>(optionConfigEntity);
	optionConfigTag->name = "TitleOptionMenuConfig";

	const auto whiteTexture = NoEngine::TextureManager::LoadCovertTexture("resources/engine/white1x1.png");
	CreateOptionSprites(whiteTexture);
}

void TitleScene::NotSystemUpdate() {
#ifdef USE_IMGUI
	No::Registry& registry = *GetRegistry();
	OptionStateComponent* optionState = nullptr;
	auto optionStateView = registry.View<CBOptionStateTag, OptionStateComponent>();
	for (auto entity : optionStateView) {
		optionState = registry.GetComponent<OptionStateComponent>(entity);
		if (optionState) {
			break;
		}
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

#endif // USE_IMGUI
}

void TitleScene::CreateOptionSprites(const NoEngine::TextureRef& whiteTexture)
{
	No::Registry& registry = *GetRegistry();
	optionItemEntities_.fill(No::nullEntity);
	optionLabelEntities_.fill(No::nullEntity);
	optionBarBaseEntities_.fill(No::nullEntity);
	optionBarFillEntities_.fill(No::nullEntity);

	auto optionViewEntity = registry.GenerateEntity();
	registry.AddComponent<CBOptionViewTag>(optionViewEntity);
	auto* optionView = registry.AddComponent<OptionMenuViewComponent>(optionViewEntity);

	const auto optionTitleTexture = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_3105/Sprite/OptionMenu.png");
	const auto masterTexture = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_3105/Sprite/Master.png");
	const auto bgmTexture = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_3105/Sprite/BGM.png");
	const auto seTexture = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_3105/Sprite/SE.png");
	const auto vibrationTexture = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_3105/Sprite/Vibration.png");
	const auto backTexture = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_3105/Sprite/Back.png");
	const auto onTexture = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_3105/Sprite/On.png");
	const auto offTexture = NoEngine::TextureManager::LoadCovertTexture("resources/game/td_3105/Sprite/Off.png");

	optionDimEntity_ = registry.GenerateEntity();
	auto* dimTransform = registry.AddComponent<No::Transform2DComponent>(optionDimEntity_);
	dimTransform->translate = { 640.0f, 360.0f };
	dimTransform->scale = { 1280.0f, 720.0f };
	auto* dimSprite = registry.AddComponent<No::SpriteComponent>(optionDimEntity_);
	dimSprite->textureHandle = whiteTexture;
	dimSprite->isVisible = false;
	dimSprite->color = { 0.0f, 0.0f, 0.0f, 0.0f };

	optionBgEntity_ = registry.GenerateEntity();
	auto* bgTransform = registry.AddComponent<No::Transform2DComponent>(optionBgEntity_);
	bgTransform->translate = { 640.0f, 400.0f };
	bgTransform->scale = { 960.0f, 520.0f };
	auto* bgSprite = registry.AddComponent<No::SpriteComponent>(optionBgEntity_);
	bgSprite->textureHandle = whiteTexture;
	bgSprite->isVisible = false;
	bgSprite->color = { 0.06f, 0.06f, 0.08f, 0.0f };

	optionLineEntity_ = registry.GenerateEntity();
	auto* lineTransform = registry.AddComponent<No::Transform2DComponent>(optionLineEntity_);
	lineTransform->translate = { 640.0f, 220.0f };
	lineTransform->scale = { 920.0f, 6.0f };
	auto* lineSprite = registry.AddComponent<No::SpriteComponent>(optionLineEntity_);
	lineSprite->textureHandle = whiteTexture;
	lineSprite->isVisible = false;
	lineSprite->color = { 1.0f, 1.0f, 1.0f, 0.0f };
	lineSprite->layer = 0;

	optionTitleEntity_ = registry.GenerateEntity();
	auto* titleTransform = registry.AddComponent<No::Transform2DComponent>(optionTitleEntity_);
	titleTransform->translate = { 920.0f, 120.0f };
	titleTransform->scale = { 280.0f, 58.0f };
	auto* titleSprite = registry.AddComponent<No::SpriteComponent>(optionTitleEntity_);
	titleSprite->textureHandle = optionTitleTexture;
	titleSprite->isVisible = false;
	titleSprite->color = { 1.0f, 1.0f, 1.0f, 0.0f };
	titleSprite->layer = 0;

	for (size_t i = 0; i < optionItemEntities_.size(); ++i) {
		optionItemEntities_[i] = registry.GenerateEntity();
		auto* itemTransform = registry.AddComponent<No::Transform2DComponent>(optionItemEntities_[i]);
		itemTransform->translate = { 640.0f, 300.0f + static_cast<float>(i) * 72.0f };
		itemTransform->scale = { 760.0f, 60.0f };
		auto* itemSprite = registry.AddComponent<No::SpriteComponent>(optionItemEntities_[i]);
		itemSprite->textureHandle = whiteTexture;
		itemSprite->isVisible = false;
		itemSprite->color = { 0.20f, 0.20f, 0.24f, 0.0f };
	}

	const std::array<NoEngine::TextureRef, 5> labelTextures = {
		masterTexture,
		bgmTexture,
		seTexture,
		vibrationTexture,
		backTexture
	};
	for (size_t i = 0; i < optionLabelEntities_.size(); ++i) {
		optionLabelEntities_[i] = registry.GenerateEntity();
		auto* tr = registry.AddComponent<No::Transform2DComponent>(optionLabelEntities_[i]);
		tr->translate = { 760.0f, 300.0f + static_cast<float>(i) * 72.0f };
		tr->scale = { 250.0f, 42.0f };
		auto* sp = registry.AddComponent<No::SpriteComponent>(optionLabelEntities_[i]);
		sp->textureHandle = labelTextures[i];
		sp->isVisible = false;
		sp->color = { 1.0f, 1.0f, 1.0f, 0.0f };
	}

	for (size_t i = 0; i < optionBarBaseEntities_.size(); ++i) {
		optionBarBaseEntities_[i] = registry.GenerateEntity();
		auto* barBaseTransform = registry.AddComponent<No::Transform2DComponent>(optionBarBaseEntities_[i]);
		barBaseTransform->translate = { 800.0f, 300.0f + static_cast<float>(i) * 72.0f };
		barBaseTransform->scale = { 360.0f, 16.0f };
		auto* barBaseSprite = registry.AddComponent<No::SpriteComponent>(optionBarBaseEntities_[i]);
		barBaseSprite->textureHandle = whiteTexture;
		barBaseSprite->isVisible = false;
		barBaseSprite->color = { 0.10f, 0.10f, 0.10f, 0.0f };

		optionBarFillEntities_[i] = registry.GenerateEntity();
		auto* barFillTransform = registry.AddComponent<No::Transform2DComponent>(optionBarFillEntities_[i]);
		barFillTransform->translate = { 800.0f, 300.0f + static_cast<float>(i) * 72.0f };
		barFillTransform->scale = { 180.0f, 16.0f };
		auto* barFillSprite = registry.AddComponent<No::SpriteComponent>(optionBarFillEntities_[i]);
		barFillSprite->textureHandle = whiteTexture;
		barFillSprite->isVisible = false;
		barFillSprite->color = { 0.95f, 0.90f, 0.28f, 0.0f };
	}

	optionToggleEntity_ = registry.GenerateEntity();
	auto* toggleTransform = registry.AddComponent<No::Transform2DComponent>(optionToggleEntity_);
	toggleTransform->translate = { 810.0f, 300.0f + 3.0f * 72.0f };
	toggleTransform->scale = { 154.7f, 34.8f };
	auto* toggleSprite = registry.AddComponent<No::SpriteComponent>(optionToggleEntity_);
	toggleSprite->textureHandle = whiteTexture;
	toggleSprite->isVisible = false;
	toggleSprite->color = { 0.20f, 0.75f, 0.30f, 0.0f };

	optionToggleOnEntity_ = registry.GenerateEntity();
	auto* onTransform = registry.AddComponent<No::Transform2DComponent>(optionToggleOnEntity_);
	onTransform->translate = { 810.0f, 300.0f + 3.0f * 72.0f };
	onTransform->scale = { 154.7f, 34.8f };
	auto* onSprite = registry.AddComponent<No::SpriteComponent>(optionToggleOnEntity_);
	onSprite->textureHandle = onTexture;
	onSprite->isVisible = false;
	onSprite->color = { 1.0f, 1.0f, 1.0f, 0.0f };

	optionToggleOffEntity_ = registry.GenerateEntity();
	auto* offTransform = registry.AddComponent<No::Transform2DComponent>(optionToggleOffEntity_);
	offTransform->translate = { 810.0f, 300.0f + 3.0f * 72.0f };
	offTransform->scale = { 154.7f, 34.8f };
	auto* offSprite = registry.AddComponent<No::SpriteComponent>(optionToggleOffEntity_);
	offSprite->textureHandle = offTexture;
	offSprite->isVisible = false;
	offSprite->color = { 1.0f, 1.0f, 1.0f, 0.0f };

	optionCursorEntity_ = registry.GenerateEntity();
	auto* cursorTransform = registry.AddComponent<No::Transform2DComponent>(optionCursorEntity_);
	cursorTransform->translate = { 680.0f, 300.0f };
	cursorTransform->scale = { 14.0f, 48.0f };
	auto* cursorSprite = registry.AddComponent<No::SpriteComponent>(optionCursorEntity_);
	cursorSprite->textureHandle = whiteTexture;
	cursorSprite->isVisible = false;
	cursorSprite->color = { 1.0f, 1.0f, 1.0f, 0.0f };

	optionView->dimEntity = optionDimEntity_;
	optionView->bgEntity = optionBgEntity_;
	optionView->lineEntity = optionLineEntity_;
	optionView->titleEntity = optionTitleEntity_;
	optionView->itemEntities = optionItemEntities_;
	optionView->labelEntities = optionLabelEntities_;
	optionView->barBaseEntities = optionBarBaseEntities_;
	optionView->barFillEntities = optionBarFillEntities_;
	optionView->toggleEntity = optionToggleEntity_;
	optionView->toggleOnEntity = optionToggleOnEntity_;
	optionView->toggleOffEntity = optionToggleOffEntity_;
	optionView->cursorEntity = optionCursorEntity_;
}
