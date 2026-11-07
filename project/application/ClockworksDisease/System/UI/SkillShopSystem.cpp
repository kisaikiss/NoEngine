#include "stdafx.h"
#include "SkillShopSystem.h"
#include "../../Component/Player/PlayerComponent.h"
#include "../../Component/Player/SkillProgressComponent.h"
#include "engine/Runtime/GraphicsCore.h"

REFLECT_STRUCT_BEGIN(SkillShopNpcTag, "ApplicationTag")
REFLECT_STRUCT_END(SkillShopNpcTag)

namespace {
constexpr char kCatalogPath[] = "resources/game/ClockworksDisease/Skills/SkillCatalog.json";
constexpr char kWhiteTexture[] = "resources/engine/white1x1.png";
constexpr int kLayer = 210000;

struct SkillDefinition {
	std::string name;
	uint32_t cost;
	std::string iconTexture;
	std::string previewTexture;
	std::string description;
};

const std::optional<SkillDefinition>& GetAirDashDefinition() {
	static const auto definition = []() -> std::optional<SkillDefinition> {
		std::ifstream file(kCatalogPath);
		try {
			const auto catalog = nlohmann::json::parse(file);
			const auto& entry = catalog.at(PlayerSkills::AirDashId);
			const auto cost = entry.at("cost").get<int>();
			if (cost <= 0) throw std::runtime_error("Skill cost must be positive");
			return SkillDefinition{
				entry.at("name").get<std::string>(), static_cast<uint32_t>(cost),
				entry.at("iconTexture").get<std::string>(), entry.at("previewTexture").get<std::string>(),
				entry.at("description").get<std::string>()
			};
		} catch (const std::exception& error) {
			LogWarning(std::string("Failed to load skill catalog: ") + error.what());
			return std::nullopt;
		}
	}();
	return definition;
}

enum class PurchaseStatus { kAvailable, kPurchased, kInsufficientPoints };

PurchaseStatus GetPurchaseStatus(const SkillProgressComponent& progress, uint32_t cost) {
	if (PlayerSkills::HasPurchased(progress, PlayerSkills::AirDashId)) return PurchaseStatus::kPurchased;
	if (progress.skillPoints < cost) return PurchaseStatus::kInsufficientPoints;
	return PurchaseStatus::kAvailable;
}

void TryPurchaseAirDash(No::Registry& registry, No::Entity player, SkillProgressComponent& progress, uint32_t cost) {
	if (GetPurchaseStatus(progress, cost) != PurchaseStatus::kAvailable) return;
	progress.purchasedSkillIds.emplace_back(PlayerSkills::AirDashId);
	progress.skillPoints -= cost;
	PlayerSkills::ApplyPurchasedAbilities(registry, player);
}

void SetPaused(No::Registry& registry, bool paused) {
	for (auto e : registry.View<No::PauseComponent>()) {
		registry.GetComponent<No::PauseComponent>(e)->isPause = paused;
	}
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

	auto* progress = registry.GetComponent<SkillProgressComponent>(player_);
	if (!progress || No::InputIsTrigger("Cancel")) {
		Close(registry);
		return;
	}

	UpdateLayout(registry);
	// 会話を送ったボタンを離すまで購入入力を受け付けない。
	if (!purchaseInputReady_) {
		purchaseInputReady_ = !No::InputIsPress("Choise");
		return;
	}
	if (No::InputIsTrigger("Choise")) {
		TryPurchaseAirDash(registry, player_, *progress, GetAirDashDefinition()->cost);
		RefreshLabels(registry, *progress);
	}
}

bool SkillShopSystem::Open(No::Registry& registry, No::Entity player) {
	const auto* progress = registry.GetComponent<SkillProgressComponent>(player);
	const auto& skill = GetAirDashDefinition();
	if (!progress || !skill) return false;

	player_ = player;
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
	};
	auto label = [&](No::Vector2 position, float scale, const std::string& text, const No::Color& color) {
		const auto entity = registry.GenerateEntity();
		registry.AddComponent<No::Transform2DComponent>(entity);
		auto* component = registry.AddComponent<No::TextComponent>(entity);
		component->fontName = "resources/engine/fonts/ISE-FONT4/Isego.otf";
		component->text = text;
		component->space = NoEngine::SpriteSpace::Screen;
		component->anchor = { 0.5f, 0.5f };
		component->vAlign = No::TextComponent::VAlign::kTop;
		component->color = color;
		component->layer = kLayer + 4;
		widgets_.push_back({ entity, position, {}, scale });
		return entity;
	};

	const No::Color gold(1.f, 0.79f, 0.36f, 1.f);
	const No::Color muted(0.70f, 0.79f, 0.86f, 1.f);
	sprite({}, {}, No::Color(0.02f, 0.03f, 0.05f, 0.78f), kLayer);
	sprite({}, { 1060.f, 550.f }, No::Color(0.06f, 0.11f, 0.17f, 1.f), kLayer + 1);
	sprite({ 0.f, -274.f }, { 1060.f, 2.f }, gold, kLayer + 2);
	sprite({ -291.f, 0.f }, { 374.f, 332.f }, No::Color(0.09f, 0.17f, 0.23f, 1.f), kLayer + 2);
	sprite({ -401.f, -37.f }, { 112.f, 112.f }, gold, kLayer + 2);
	sprite({ -401.f, -37.f }, { 104.f, 104.f }, No::Color::WHITE, kLayer + 3, skill->iconTexture);
	sprite({ 198.f, -53.f }, { 540.f, 240.f }, No::Color::WHITE, kLayer + 3, skill->previewTexture);

	label({ -475.f, -242.f }, 0.9f, "スキルショップ", No::Color::WHITE);
	pointsLabel_ = label({ 280.f, -230.f }, 0.63f, "", gold);
	label({ -455.f, -148.f }, 0.48f, "販売スキル", muted);
	label({ -331.f, -72.f }, 0.62f, skill->name, No::Color::WHITE);
	label({ -331.f, -31.f }, 0.52f, std::to_string(skill->cost) + " SP", gold);
	statusLabel_ = label({ -455.f, 97.f }, 0.55f, "", muted);
	label({ -72.f, 85.f }, 0.73f, skill->name, No::Color::WHITE);
	label({ -72.f, 133.f }, 0.52f, skill->description, muted);
	controlsLabel_ = label({ -475.f, 215.f }, 0.52f, "", gold);
	label({ 210.f, 215.f }, 0.52f, "B / Backspace  閉じる", muted);

	UpdateLayout(registry);
	RefreshLabels(registry, *progress);
	return true;
}

void SkillShopSystem::Close(No::Registry& registry) {
	for (const auto& widget : widgets_) registry.DestroyEntity(widget.entity);
	widgets_.clear();
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

void SkillShopSystem::RefreshLabels(No::Registry& registry, const SkillProgressComponent& progress) const {
	registry.GetComponent<No::TextComponent>(pointsLabel_)->text = "所持SP  " + std::to_string(progress.skillPoints);
	auto* status = registry.GetComponent<No::TextComponent>(statusLabel_);
	auto* controls = registry.GetComponent<No::TextComponent>(controlsLabel_);
	switch (GetPurchaseStatus(progress, GetAirDashDefinition()->cost)) {
	case PurchaseStatus::kAvailable:
		status->text = "購入できます";
		controls->text = "A / Space  購入";
		break;
	case PurchaseStatus::kPurchased:
		status->text = "習得済み";
		controls->text.clear();
		break;
	case PurchaseStatus::kInsufficientPoints:
		status->text = "SPが足りません";
		controls->text.clear();
		break;
	}
}

REGISTER_SYSTEM(::SkillShopSystem, "SkillShopSystem", "UI")
