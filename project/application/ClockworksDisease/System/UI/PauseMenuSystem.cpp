#include "stdafx.h"
#include "PauseMenuSystem.h"
#include "application/ClockworksDisease/Component/UI/PauseMenuComponent.h"
#include "application/ClockworksDisease/Component/Camera/FollowCameraComponent.h"
#include "application/ClockworksDisease/Component/Game/GameQuitEvent.h"
#include "engine/Assets/Font/FontManager.h"
#include "engine/Runtime/GraphicsCore.h"

namespace {
constexpr float kMoveSpeedStep = 0.5f;
constexpr float kMoveSpeedMin = 0.5f;
constexpr float kMoveSpeedMax = 10.0f;

const No::Color kNormalColor = No::Color(1.0f, 1.0f, 1.0f, 1.0f);
const No::Color kSelectedColor = No::Color(1.0f, 0.85f, 0.2f, 1.0f);

const char* kMainItemLabels[PauseMenuSystem::kMainItemCount == 0 ? 0 : 4] = { "Resume", "Option", "Title", "Quit" };
}

void PauseMenuSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);

	if (!initialized_) {
		CreateMenuEntities(registry);
		initialized_ = true;
	}

	auto* menu = registry.GetComponent<PauseMenuComponent>(menuEntity_);
	if (!menu) return;

	switch (menu->state) {
	case PauseMenuState::kClosed:
		if (No::InputIsTrigger("Pause")) {
			OpenMenu(registry);
		}
		break;
	case PauseMenuState::kMain:
		UpdateMainMenu(registry);
		break;
	case PauseMenuState::kOption:
		UpdateOptionMenu(registry);
		break;
	}

	No::Vector2 windowSize = NoEngine::GraphicsCore::GetWindowSize();
	auto* t2d = registry.AddComponent<No::Transform2DComponent>(backgroundEntity_);
	t2d->scale = windowSize;
	t2d->translate = windowSize / 2.f;
}

void PauseMenuSystem::CreateMenuEntities(No::Registry& registry) {
	No::Vector2 windowSize = NoEngine::GraphicsCore::GetWindowSize();

	// 管理用Entity
	menuEntity_ = registry.GenerateEntity();
	registry.AddComponent<PauseMenuComponent>(menuEntity_);
	registry.AddComponent<No::EditTag>(menuEntity_)->name = "PauseMenu";

	auto font = NoEngine::FontManager::LoadFontFile("resources/engine/fonts/UbuntuMono-R.ttf");

	// 背景(画面全体を覆う半透明の黒)
	{
		backgroundEntity_ = registry.GenerateEntity();
		auto* t2d = registry.AddComponent<No::Transform2DComponent>(backgroundEntity_);
		t2d->scale = windowSize;
		t2d->translate = windowSize / 2.f;
		auto* sprite = registry.AddComponent<No::SpriteComponent>(backgroundEntity_);
		sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/engine/white1x1.png");
		sprite->color = No::Color(0.f, 0.f, 0.f, 0.6f);
		sprite->space = NoEngine::SpriteSpace::Screen;
		sprite->layer = 200000;
		sprite->isVisible = false;
	}

	auto createText = [&](const std::string& text, float y, float scale) {
		auto e = registry.GenerateEntity();
		auto* t2d = registry.AddComponent<No::Transform2DComponent>(e);
		t2d->translate = { 0.f, y };
		auto* txt = registry.AddComponent<No::TextComponent>(e);
		txt->fontHandle = font;
		txt->text = text;
		txt->scale = scale;
		txt->hAlign = No::TextComponent::HAlign::kCenter;
		txt->vAlign = No::TextComponent::VAlign::kMiddle;
		txt->space = NoEngine::SpriteSpace::Screen;
		txt->anchor = { 0.5f, 0.5f };
		txt->layer = 200001;
		txt->isVisible = false;
		return e;
		};

	// メイン画面
	mainTitleEntity_ = createText("PAUSE", -180.f, 2.0f);
	for (int i = 0; i < kMainItemCount; ++i) {
		mainItemEntities_[i] = createText(kMainItemLabels[i], -60.f + static_cast<float>(i) * 60.f, 1.2f);
		registry.AddComponent<PauseMenuItemTag>(mainItemEntities_[i])->index = i;
	}

	// オプション画面
	optionTitleEntity_ = createText("OPTION", -180.f, 2.0f);
	optionLabelEntity_ = createText("Camera Move Speed", -40.f, 1.2f);
	optionValueEntity_ = createText("", 20.f, 1.2f);
	optionHintEntity_ = createText("<-/-> : Change   Backspace/B Button : Back", 120.f, 0.8f);
}

void PauseMenuSystem::OpenMenu(No::Registry& registry) {
	auto* menu = registry.GetComponent<PauseMenuComponent>(menuEntity_);
	menu->state = PauseMenuState::kMain;
	menu->mainSelectedIndex = 0;
	SetPause(registry, true);
	RefreshMainMenuVisuals(registry);
}

void PauseMenuSystem::CloseMenu(No::Registry& registry) {
	auto* menu = registry.GetComponent<PauseMenuComponent>(menuEntity_);
	menu->state = PauseMenuState::kClosed;
	SetPause(registry, false);

	if (auto* sprite = registry.GetComponent<No::SpriteComponent>(backgroundEntity_)) sprite->isVisible = false;
	if (auto* t = registry.GetComponent<No::TextComponent>(mainTitleEntity_)) t->isVisible = false;
	for (auto e : mainItemEntities_) {
		if (auto* t = registry.GetComponent<No::TextComponent>(e)) t->isVisible = false;
	}
	if (auto* t = registry.GetComponent<No::TextComponent>(optionTitleEntity_)) t->isVisible = false;
	if (auto* t = registry.GetComponent<No::TextComponent>(optionLabelEntity_)) t->isVisible = false;
	if (auto* t = registry.GetComponent<No::TextComponent>(optionValueEntity_)) t->isVisible = false;
	if (auto* t = registry.GetComponent<No::TextComponent>(optionHintEntity_)) t->isVisible = false;
}

void PauseMenuSystem::UpdateMainMenu(No::Registry& registry) {
	auto* menu = registry.GetComponent<PauseMenuComponent>(menuEntity_);

	if (No::InputIsTrigger("Pause")) {
		CloseMenu(registry);
		return;
	}

	if (No::InputIsTrigger("Down")) {
		menu->mainSelectedIndex = (menu->mainSelectedIndex + 1) % kMainItemCount;
	}
	if (No::InputIsTrigger("Up")) {
		menu->mainSelectedIndex = (menu->mainSelectedIndex + kMainItemCount - 1) % kMainItemCount;
	}

	if (No::InputIsTrigger("Choise")) {
		switch (menu->mainSelectedIndex) {
		case 0: // Resume
			CloseMenu(registry);
			return;
		case 1: // Option
			menu->state = PauseMenuState::kOption;
			RefreshOptionMenuVisuals(registry);
			return;
		case 2: { // タイトルへ戻る
			No::SceneChangeEvent sceneChangeEvent;
			sceneChangeEvent.nextScene = "TitleScene";
			registry.EmitEvent(sceneChangeEvent);
			CloseMenu(registry);
			return;
		}
		case 3: // ゲームをやめる
			registry.EmitEvent(GameQuitEvent{});
			return;
		default:
			break;
		}
	}

	RefreshMainMenuVisuals(registry);
}

void PauseMenuSystem::UpdateOptionMenu(No::Registry& registry) {
	auto* menu = registry.GetComponent<PauseMenuComponent>(menuEntity_);

	if (No::InputIsTrigger("Cancel") || No::InputIsTrigger("Pause")) {
		menu->state = PauseMenuState::kMain;
		RefreshMainMenuVisuals(registry);
		return;
	}

	for (auto e : registry.View<FollowCameraComponent>()) {
		auto* fc = registry.GetComponent<FollowCameraComponent>(e);
		if (No::InputIsTrigger("Right")) {
			fc->moveSpeed = std::min(fc->moveSpeed + kMoveSpeedStep, kMoveSpeedMax);
		}
		if (No::InputIsTrigger("Left")) {
			fc->moveSpeed = std::max(fc->moveSpeed - kMoveSpeedStep, kMoveSpeedMin);
		}
	}

	RefreshOptionMenuVisuals(registry);
}

void PauseMenuSystem::RefreshMainMenuVisuals(No::Registry& registry) {
	auto* menu = registry.GetComponent<PauseMenuComponent>(menuEntity_);

	if (auto* sprite = registry.GetComponent<No::SpriteComponent>(backgroundEntity_)) sprite->isVisible = true;
	if (auto* t = registry.GetComponent<No::TextComponent>(mainTitleEntity_)) t->isVisible = true;

	for (int i = 0; i < kMainItemCount; ++i) {
		auto* t = registry.GetComponent<No::TextComponent>(mainItemEntities_[i]);
		if (!t) continue;
		t->isVisible = true;
		t->color = (i == menu->mainSelectedIndex) ? kSelectedColor : kNormalColor;
	}

	if (auto* t = registry.GetComponent<No::TextComponent>(optionTitleEntity_)) t->isVisible = false;
	if (auto* t = registry.GetComponent<No::TextComponent>(optionLabelEntity_)) t->isVisible = false;
	if (auto* t = registry.GetComponent<No::TextComponent>(optionValueEntity_)) t->isVisible = false;
	if (auto* t = registry.GetComponent<No::TextComponent>(optionHintEntity_)) t->isVisible = false;
}

void PauseMenuSystem::RefreshOptionMenuVisuals(No::Registry& registry) {
	if (auto* sprite = registry.GetComponent<No::SpriteComponent>(backgroundEntity_)) sprite->isVisible = true;

	if (auto* t = registry.GetComponent<No::TextComponent>(mainTitleEntity_)) t->isVisible = false;
	for (auto e : mainItemEntities_) {
		if (auto* t = registry.GetComponent<No::TextComponent>(e)) t->isVisible = false;
	}

	if (auto* t = registry.GetComponent<No::TextComponent>(optionTitleEntity_)) t->isVisible = true;
	if (auto* t = registry.GetComponent<No::TextComponent>(optionLabelEntity_)) t->isVisible = true;
	if (auto* t = registry.GetComponent<No::TextComponent>(optionHintEntity_)) t->isVisible = true;

	float moveSpeed = 0.f;
	for (auto e : registry.View<FollowCameraComponent>()) {
		moveSpeed = registry.GetComponent<FollowCameraComponent>(e)->moveSpeed;
	}
	if (auto* t = registry.GetComponent<No::TextComponent>(optionValueEntity_)) {
		t->isVisible = true;
		t->text = std::format("{:.1f}", moveSpeed);
	}
}

void PauseMenuSystem::SetPause(No::Registry& registry, bool isPause) {
	for (auto e : registry.View<No::PauseComponent>()) {
		registry.GetComponent<No::PauseComponent>(e)->isPause = isPause;
	}
}