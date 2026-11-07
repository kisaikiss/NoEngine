#pragma once
#include "engine/NoEngine.h"

struct SkillShopNpcTag {};

struct OpenSkillShopEvent {
	No::Entity player = No::INVALID_ENTITY;
};

struct SkillProgressComponent;

class SkillShopSystem : public No::ISystem {
public:
	SkillShopSystem() { SetStopInPause(false); }
	void Update(No::Registry& registry, float deltaTime) override;

private:
	struct Widget {
		No::Entity entity;
		No::Vector2 position;
		No::Vector2 size;
		float textScale = 0.f;
	};
	struct Tile {
		No::Entity frame;
		No::Entity icon;
		No::Entity badge;
	};

	No::Entity player_ = No::INVALID_ENTITY;
	No::Entity pointsLabel_ = No::INVALID_ENTITY;
	No::Entity staminaLabel_ = No::INVALID_ENTITY;
	No::Entity preview_ = No::INVALID_ENTITY;
	No::Entity nameLabel_ = No::INVALID_ENTITY;
	No::Entity costLabel_ = No::INVALID_ENTITY;
	No::Entity descriptionLabel_ = No::INVALID_ENTITY;
	No::Entity statusLabel_ = No::INVALID_ENTITY;
	No::Entity controlsLabel_ = No::INVALID_ENTITY;
	std::vector<Widget> widgets_;
	std::vector<std::vector<Tile>> tiles_;
	size_t selectedColumn_ = 0;
	size_t selectedRow_ = 0;
	bool purchaseInputReady_ = false;

	bool Open(No::Registry& registry, No::Entity player);
	void Close(No::Registry& registry);
	void UpdateLayout(No::Registry& registry) const;
	bool UpdateSelection();
	void RefreshSelection(No::Registry& registry) const;
	void RefreshPurchaseState(No::Registry& registry, const SkillProgressComponent& progress) const;
};
