#include "ScoreSpriteControlSystem.h"
#include "application/TD2_3/Component/PlayerstatusComponent.h"
#include "application/TD2_3/Component/ScoreDigitComponent.h"

namespace {
const NoEngine::Vector2 skScorePositionBase = { 1230.f,120.f };
const float skSpriteWidth = 64.f;
const uint32_t skDigitNum = 6;
}

void ScoreSpriteControlSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	auto PlayerStatusView = registry.View<PlayerStatusComponent>();

	uint32_t score = 0;
	for (auto entity : PlayerStatusView) {
		auto* status = registry.GetComponent<PlayerStatusComponent>(entity);
		score = status->score;
	}

	std::array<uint32_t, skDigitNum> digits_;
	for (uint32_t i = 0; i < skDigitNum; i++) {
		digits_[i] = score % 10;
		score /= 10;
	}

	auto view = registry.View<No::Transform2DComponent, No::SpriteComponent, ScoreDigitComponent>();
	uint32_t counter = 0;
	for (auto entity : view) {
		auto* sprite = registry.GetComponent<No::SpriteComponent>(entity);

		sprite->uv.x = 0.1f * digits_[counter];
		auto* transform = registry.GetComponent<No::Transform2DComponent>(entity);
		NoEngine::Vector2 offset = { static_cast<float>(counter) * skSpriteWidth, 0.f };
		transform->translate = skScorePositionBase - offset;


		counter++;
	}
}
