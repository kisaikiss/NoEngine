#include "stdafx.h"
#include "GameClearScene.h"

#include "../System/Game/GameResult.h"
#include "../Component/UI/NumberDisplayComponent.h"
#include "../System/UI/NumberDisplaySystem.h"
#include "engine/Assets/Font/FontManager.h"

namespace {

// digitCount桁分のスプライトEntityを横並びで生成し、NumberDisplayComponentを付けたEntityを返す
No::Entity CreateNumberDisplay(No::Registry& registry, const No::Vector2& position,
	uint32_t digitCount, float digitWidth, float digitHeight, uint32_t layer) {

	auto owner = registry.GenerateEntity();
	auto* display = registry.AddComponent<NumberDisplayComponent>(owner);
	registry.AddComponent<No::Transform2DComponent>(owner);
	display->digitCount = digitCount;

	for (uint32_t i = 0; i < digitCount; ++i) {
		auto digitEntity = registry.GenerateEntity();

		auto* t2d = registry.AddComponent<No::Transform2DComponent>(digitEntity);
		t2d->scale = { digitWidth, digitHeight };
		t2d->translate = { position.x + digitWidth * static_cast<float>(i), position.y };
		t2d->parent = owner;
		auto* sprite = registry.AddComponent<No::SpriteComponent>(digitEntity);
		sprite->textureName = "numbers";
		sprite->layer = layer;
		sprite->uv.width = 1.0f / 10.0f;

		registry.AddComponent<NumberDigitTag>(digitEntity)->digitIndex = i;

		display->digitEntities.push_back(digitEntity);
	}
	return owner;
}

} // namespace

void GameClearScene::Setup() {
	AddSystems();

	auto& registry = *GetRegistry();
	const auto& result = GameResult::Get();

	// アイテム数 "ITEM  取得数/総数"
	{

		auto collected = CreateNumberDisplay(registry, No::Vector2::ZERO, 3, 32.f, 40.f, 1);
		registry.GetComponent<NumberDisplayComponent>(collected)->value = result.collectedItemCount;
		registry.AddComponent<No::EditTag>(collected)->name = "collected";


		auto total = CreateNumberDisplay(registry, No::Vector2::ZERO, 3, 32.f, 40.f, 1);
		registry.GetComponent<NumberDisplayComponent>(total)->value = result.totalItemCount;
		registry.AddComponent<No::EditTag>(total)->name = "total";
	}

	// クリアタイム "TIME  分:秒"
	{

		uint32_t totalSeconds = static_cast<uint32_t>(result.clearTime);
		uint32_t minutes = totalSeconds / 60;
		uint32_t seconds = totalSeconds % 60;

		auto minuteDisplay = CreateNumberDisplay(registry, No::Vector2::ZERO , 2, 32.f, 40.f, 1);
		registry.GetComponent<NumberDisplayComponent>(minuteDisplay)->value = minutes;
		registry.AddComponent<No::EditTag>(minuteDisplay)->name = "ClearMinutes";


		auto secondDisplay = CreateNumberDisplay(registry, No::Vector2::ZERO, 2, 32.f, 40.f, 1);
		registry.GetComponent<NumberDisplayComponent>(secondDisplay)->value = seconds;
		registry.AddComponent<No::EditTag>(secondDisplay)->name = "ClearSeconds";
	}
}

void GameClearScene::AddSystems() {
	AddSystem(std::make_unique<No::EditSystem>());

	AddSystem(std::make_unique<No::ModelLoadSystem>());
	AddSystem(std::make_unique<No::SpriteLoadSystem>());
	AddSystem(std::make_unique<No::AnimationSystem>());

	AddSystem(std::make_unique<No::EffectEmitSystem>());
	AddSystem(std::make_unique<No::ParticleEmitterSystem>());
	AddSystem(std::make_unique<No::ParticleSystem>());

	AddSystem(std::make_unique<No::MovementSystem>());
	AddSystem(std::make_unique<No::TransformRoutineSystem>());

	AddSystem(std::make_unique<No::SpriteAnimationSystem>());

	AddSystem(std::make_unique<No::DrawManipulatorSystem>());
	AddSystem(std::make_unique<No::DebugCameraSystem>());
	AddSystem(std::make_unique<No::CameraSystem>());
	AddSystem(std::make_unique<No::DrawCameraFrustumSystem>());
	AddSystem(std::make_unique<NumberDisplaySystem>());
}