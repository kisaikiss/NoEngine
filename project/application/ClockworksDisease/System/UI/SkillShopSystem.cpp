#include "stdafx.h"
#include "SkillShopSystem.h"
#include "../../Component/Player/SkillProgressComponent.h"
#include "engine/Runtime/GraphicsCore.h"

REFLECT_STRUCT_BEGIN(SkillShopNpcTag, "ApplicationTag")
REFLECT_STRUCT_END(SkillShopNpcTag)

namespace {
constexpr char kWhiteTexture[] = "resources/engine/white1x1.png";
constexpr int kLayer = 210000;
const No::Color kGold(1.f, 0.79f, 0.36f, 1.f);
const No::Color kMuted(0.70f, 0.79f, 0.86f, 1.f);
const No::Color kPurchased(0.56f, 0.88f, 0.73f, 1.f);

void SetPaused(No::Registry& registry, bool paused) {
	for (auto e : registry.View<No::PauseComponent>()) {
		registry.GetComponent<No::PauseComponent>(e)->isPause = paused;
	}
}

int NavigationStep(const char* axis, const char* negative, const char* positive) {
	const float now = No::GetInputAxisValue(axis);
	const float before = No::GetInputPreAxisValue(axis);
	const bool left = No::InputIsTrigger(negative) || (now < 0.f && before >= 0.f);
	const bool right = No::InputIsTrigger(positive) || (now > 0.f && before <= 0.f);
	return static_cast<int>(right) - static_cast<int>(left);
}

size_t MoveIndex(size_t index, size_t count, int step) {
	if (step < 0 && index > 0) return index - 1;
	if (step > 0 && index + 1 < count) return index + 1;
	return index;
}
}

void SkillShopSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	if (player_ == No::INVALID_ENTITY) {
		for (auto e : registry.View<No::PauseComponent>()) {
			if (registry.GetComponent<No::PauseComponent>(e)->isPause) return;
		}
		for (const auto& event : registry.PollAllEvents<OpenSkillShopEvent>()) {
			if (Open(registry, event.player)) break;
		}
		return;
	}

	const auto* progress = registry.GetComponent<SkillProgressComponent>(player_);
	if (!progress || !registry.Has<PlayerComponent>(player_) || No::InputIsTrigger("Cancel")) {
		Close(registry);
		return;
	}

	UpdateLayout(registry);
	if (UpdateSelection()) {
		RefreshSelection(registry);
		RefreshPurchaseState(registry, *progress);
	}

	// 会話を送ったボタンを離すまで購入入力を受け付けない。
	if (!purchaseInputReady_) {
		purchaseInputReady_ = !No::InputIsPress("Choise");
		return;
	}
	if (No::InputIsTrigger("Choise")) {
		const auto& selected = PlayerSkills::GetCatalog()[selectedColumn_].skills[selectedRow_];
		if (PlayerSkills::TryPurchase(registry, player_, selected.id)) RefreshPurchaseState(registry, *progress);
	}
}

bool SkillShopSystem::Open(No::Registry& registry, No::Entity player) {
	const auto* progress = registry.GetComponent<SkillProgressComponent>(player);
	const auto& catalog = PlayerSkills::GetCatalog();
	if (!progress || !registry.Has<PlayerComponent>(player) || catalog.empty()) return false;

	player_ = player;
	selectedColumn_ = 0;
	selectedRow_ = 0;
	purchaseInputReady_ = false;
	SetPaused(registry, true);

	auto sprite = [&](No::Vector2 position, No::Vector2 size, const No::Color& color,
		uint32_t layer, const std::string& texture = kWhiteTexture) {
		const auto entity = registry.GenerateEntity();
		registry.AddComponent<No::Transform2DComponent>(entity);
		auto* component = registry.AddComponent<No::SpriteComponent>(entity);
		component->textureHandle = NoEngine::TextureManager::LoadCovertTexture(texture);
		component->space = NoEngine::SpriteSpace::Screen;
		component->anchor = { 0.5f, 0.5f };
		component->color = color;
		component->layer = layer;
		widgets_.push_back({ entity, position, size });
		return entity;
	};

	using HAlign = No::TextComponent::HAlign;
	auto label = [&](No::Vector2 position, float scale, const std::string& text, const No::Color& color,
		HAlign align = HAlign::kLeft) {
		const auto entity = registry.GenerateEntity();
		registry.AddComponent<No::Transform2DComponent>(entity);
		auto* component = registry.AddComponent<No::TextComponent>(entity);
		component->fontName = "resources/engine/fonts/ISE-FONT4/Isego.otf";
		component->text = text;
		component->space = NoEngine::SpriteSpace::Screen;
		component->anchor = { 0.5f, 0.5f };
		component->hAlign = align;
		component->vAlign = No::TextComponent::VAlign::kTop;
		component->color = color;
		component->layer = kLayer + 5;
		widgets_.push_back({ entity, position, {}, scale });
		return entity;
	};

	sprite({}, {}, No::Color(0.02f, 0.03f, 0.05f, 0.78f), kLayer);
	sprite({}, { 1160.f, 600.f }, No::Color(0.06f, 0.11f, 0.17f, 1.f), kLayer + 1);
	sprite({ 0.f, -298.f }, { 1160.f, 3.f }, kGold, kLayer + 2);
	sprite({ -282.f, 14.f }, { 536.f, 408.f }, No::Color(0.09f, 0.17f, 0.23f, 1.f), kLayer + 2);
	sprite({ 275.f, 14.f }, { 548.f, 408.f }, No::Color(0.08f, 0.14f, 0.20f, 1.f), kLayer + 2);

	label({ -548.f, -275.f }, 0.87f, "スキルショップ", No::Color::WHITE);
	pointsLabel_ = label({ 546.f, -260.f }, 0.65f, "", kGold, HAlign::kRight);
	staminaLabel_ = label({ -24.f, -230.f }, 0.43f, "", kMuted, HAlign::kRight);

	const size_t maxRows = std::ranges::max(catalog | std::views::transform(
		[](const PlayerSkills::SkillGroup& group) { return group.skills.size(); }));
	const float columnStep = std::min(116.f, 512.f / static_cast<float>(catalog.size()));
	const float rowStep = std::min(95.f, 340.f / static_cast<float>(maxRows));
	const float tileSize = std::min({ 72.f, columnStep * 0.76f, rowStep * 0.74f });
	for (const auto& group : catalog) {
		const float x = -282.f + columnStep * (static_cast<float>(tiles_.size()) - (static_cast<float>(catalog.size()) - 1.f) * 0.5f);
		label({ x, -170.f }, std::min(0.43f, columnStep / 240.f), group.name, kMuted, HAlign::kCenter);
		auto& column = tiles_.emplace_back();
		for (const auto& skill : group.skills) {
			const float y = -134.f + rowStep * (static_cast<float>(column.size()) + 0.5f) - (rowStep - tileSize) * 0.25f;
			const auto frame = sprite({ x, y }, { tileSize, tileSize }, kMuted, kLayer + 3);
			const auto icon = sprite({ x, y }, { tileSize * 0.90f, tileSize * 0.90f }, No::Color::WHITE, kLayer + 4, skill.iconTexture);
			const auto badge = label({ x, y + tileSize * 0.50f + 1.f }, std::min(0.34f, rowStep / 245.f), "", kMuted, HAlign::kCenter);
			column.push_back({ frame, icon, badge });
		}
	}

	preview_ = sprite({ 275.f, -67.f }, { 504.f, 224.f }, No::Color::WHITE, kLayer + 3);
	nameLabel_ = label({ 23.f, 57.f }, 0.68f, "", No::Color::WHITE);
	costLabel_ = label({ 525.f, 61.f }, 0.52f, "", kGold, HAlign::kRight);
	descriptionLabel_ = label({ 23.f, 103.f }, 0.45f, "", kMuted);
	statusLabel_ = label({ 23.f, 182.f }, 0.44f, "", kGold);
	label({ -548.f, 242.f }, 0.48f, "矢印キー / 左スティック  選択", kMuted);
	controlsLabel_ = label({ 23.f, 242.f }, 0.48f, "", kGold);
	label({ 546.f, 242.f }, 0.48f, "B / Backspace  閉じる", kMuted, HAlign::kRight);

	UpdateLayout(registry);
	RefreshSelection(registry);
	RefreshPurchaseState(registry, *progress);
	return true;
}

void SkillShopSystem::Close(No::Registry& registry) {
	for (const auto& widget : widgets_) registry.DestroyEntity(widget.entity);
	widgets_.clear();
	tiles_.clear();
	player_ = No::INVALID_ENTITY;
	SetPaused(registry, false);
}

void SkillShopSystem::UpdateLayout(No::Registry& registry) const {
	const auto windowSize = NoEngine::GraphicsCore::GetWindowSize();
	const float scale = std::min(windowSize.x / 1280.f, windowSize.y / 720.f);
	for (const auto& widget : widgets_) {
		auto* transform = registry.GetComponent<No::Transform2DComponent>(widget.entity);
		transform->translate = widget.position * scale;
		if (auto* label = registry.GetComponent<No::TextComponent>(widget.entity)) {
			label->scale = widget.textScale * scale;
		} else {
			transform->scale = widget.size.x > 0.f ? widget.size * scale : windowSize;
		}
	}
}

bool SkillShopSystem::UpdateSelection() {
	const auto& catalog = PlayerSkills::GetCatalog();
	const size_t column = MoveIndex(selectedColumn_, catalog.size(), NavigationStep("MenuHorizontal", "Left", "Right"));
	const size_t rows = catalog[column].skills.size();
	const size_t row = MoveIndex(std::min(selectedRow_, rows - 1), rows, NavigationStep("MenuVertical", "Down", "Up") * -1);
	const bool changed = column != selectedColumn_ || row != selectedRow_;
	selectedColumn_ = column;
	selectedRow_ = row;
	return changed;
}

void SkillShopSystem::RefreshSelection(No::Registry& registry) const {
	const auto& skill = PlayerSkills::GetCatalog()[selectedColumn_].skills[selectedRow_];
	auto* preview = registry.GetComponent<No::SpriteComponent>(preview_);
	if (!preview->SetTextureFromPath(skill.previewTexture)) preview->textureHandle = nullptr;
	registry.GetComponent<No::TextComponent>(nameLabel_)->text = skill.name;
	registry.GetComponent<No::TextComponent>(costLabel_)->text = std::to_string(skill.cost) + " SP";
	registry.GetComponent<No::TextComponent>(descriptionLabel_)->text = skill.description;
}

void SkillShopSystem::RefreshPurchaseState(No::Registry& registry, const SkillProgressComponent& progress) const {
	using PlayerSkills::PurchaseStatus;
	const auto& catalog = PlayerSkills::GetCatalog();
	registry.GetComponent<No::TextComponent>(pointsLabel_)->text = "所持SP  " + std::to_string(progress.skillPoints);
	const float stamina = registry.GetComponent<PlayerComponent>(player_)->maxStamina;
	registry.GetComponent<No::TextComponent>(staminaLabel_)->text = std::format("最大スタミナ  {:.1f}", stamina);
	for (size_t column = 0; column < tiles_.size(); ++column) {
		for (size_t row = 0; row < tiles_[column].size(); ++row) {
			const auto& tile = tiles_[column][row];
			const auto state = PlayerSkills::GetPurchaseStatus(progress, catalog[column].skills[row]);
			const bool purchased = state == PurchaseStatus::kPurchased;
			const bool locked = state == PurchaseStatus::kStageLocked || state == PurchaseStatus::kPrerequisiteLocked;
			auto* frame = registry.GetComponent<No::SpriteComponent>(tile.frame);
			frame->color = (column == selectedColumn_ && row == selectedRow_) ? kGold : purchased ? kPurchased : kMuted;
			registry.GetComponent<No::SpriteComponent>(tile.icon)->color = locked ? No::Color(0.45f, 0.45f, 0.45f, 1.f) : No::Color::WHITE;
			auto* badge = registry.GetComponent<No::TextComponent>(tile.badge);
			badge->color = purchased ? kPurchased : kMuted;
			badge->text = purchased ? "習得済" : locked ? "未解放" : "";
		}
	}

	const auto& selected = catalog[selectedColumn_].skills[selectedRow_];
	auto* status = registry.GetComponent<No::TextComponent>(statusLabel_);
	auto* controls = registry.GetComponent<No::TextComponent>(controlsLabel_);
	controls->text.clear();
	switch (PlayerSkills::GetPurchaseStatus(progress, selected)) {
	case PurchaseStatus::kAvailable:
		status->text = "購入できます";
		controls->text = "A / Space  購入";
		break;
	case PurchaseStatus::kPurchased:
		status->text = "習得済み";
		break;
	case PurchaseStatus::kStageLocked:
		status->text = "ステージを進めると解放";
		break;
	case PurchaseStatus::kPrerequisiteLocked:
		status->text = PlayerSkills::FindMissingPrerequisite(progress, selected)->name + " の習得が必要";
		break;
	case PurchaseStatus::kInsufficientPoints:
		status->text = "SPが足りません";
		break;
	}
}

REGISTER_SYSTEM(::SkillShopSystem, "SkillShopSystem", "UI")
