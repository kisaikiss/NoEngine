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

	No::Entity player_ = No::INVALID_ENTITY;
	No::Entity pointsLabel_ = No::INVALID_ENTITY;
	No::Entity statusLabel_ = No::INVALID_ENTITY;
	No::Entity controlsLabel_ = No::INVALID_ENTITY;
	std::vector<Widget> widgets_;
	bool purchaseInputReady_ = false;

	bool Open(No::Registry& registry, No::Entity player);
	void Close(No::Registry& registry);
	void UpdateLayout(No::Registry& registry) const;
	void RefreshLabels(No::Registry& registry, const SkillProgressComponent& progress) const;
};
