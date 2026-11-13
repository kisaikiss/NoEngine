#include "stdafx.h"
#include "NumberDisplaySystem.h"
#include "../../Component/UI/NumberDisplayComponent.h"
#include "../Game/GameResult.h"

namespace {
constexpr float kDigitUVWidth = 1.0f / 10.0f;
}

void NumberDisplaySystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);

	for (auto e : registry.View<NumberDisplayComponent>()) {
		auto* display = registry.GetComponent<NumberDisplayComponent>(e);
		if (const auto* binding = registry.GetComponent<GameResultNumberComponent>(e)) {
			const auto& result = GameResult::Get();
			switch (binding->source) {
			case GameResultNumberComponent::Source::CollectedItems: display->value = result.collectedItemCount; break;
			case GameResultNumberComponent::Source::TotalItems: display->value = result.totalItemCount; break;
			case GameResultNumberComponent::Source::ClearMinutes: display->value = static_cast<uint32_t>(result.clearTime) / 60; break;
			case GameResultNumberComponent::Source::ClearSeconds: display->value = static_cast<uint32_t>(result.clearTime) % 60; break;
			}
		}

		// 上位桁から順にdigits[]へ展開する
		std::vector<uint32_t> digits(display->digitCount, 0);
		for (uint32_t i = 0; i < display->digitCount; ++i) {
			uint32_t place = display->digitCount - 1 - i;
			uint32_t divisor = 1;
			for (uint32_t p = 0; p < place; ++p) divisor *= 10;
			digits[i] = (display->value / divisor) % 10;
		}

		for (size_t i = 0; i < display->digitEntities.size() && i < digits.size(); ++i) {
			auto* sprite = registry.GetComponent<No::SpriteComponent>(display->digitEntities[i]);
			if (!sprite) continue;
			sprite->uv.width = kDigitUVWidth;
			sprite->uv.x = static_cast<float>(digits[i]) * kDigitUVWidth;
		}
	}
}

REGISTER_SYSTEM(::NumberDisplaySystem, "NumberDisplaySystem", "UI")
