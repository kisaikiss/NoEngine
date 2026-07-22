#include "stdafx.h"
#include "LevelUISystem.h"
#include "../../Component/Player/PlayerComponent.h"

REFLECT_STRUCT_BEGIN(LevelGaugeTag, "ApplicationTag")
REFLECT_STRUCT_END(LevelGaugeTag)

REFLECT_STRUCT_BEGIN(LevelNumberTag, "ApplicationTag")
REFLECT_STRUCT_END(LevelNumberTag)

void LevelUISystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	
	// 現在のパワーと次のレベルになるのに必要なパワーを収集
	float power = 0.0f;
	float nextLevelUp = 0.0f;
	uint32_t nowLevel = 0;
	for (auto e : registry.View<LevelComponent>()) {
		auto* level = registry.GetComponent<LevelComponent>(e);
		power = static_cast<float>(level->power);
		nextLevelUp = static_cast<float>(level->nextLevelUp);
		nowLevel = level->nowLevel;
	}

	// 次のレベルになるのに必要なパワーが0の場合はプレイヤーが存在しないため、早期リターンする
	if (nextLevelUp == 0.0f) {
		return;
	}

	// 収集した情報を元にレベルゲージの溜まり具合を設定
	for (auto e : registry.View<No::SpriteComponent, LevelGaugeTag>()) {
		auto* sprite = registry.GetComponent<No::SpriteComponent>(e);
		sprite->fill = power / nextLevelUp;
	}

	// 収集した情報を元に現在のレベル数の表示を設定
	for (auto e : registry.View<No::SpriteComponent, LevelNumberTag>()) {
		auto* sprite = registry.GetComponent<No::SpriteComponent>(e);
		sprite->uv.width = 1.f / 10.f;
		sprite->uv.x = static_cast<float>(nowLevel) / 10.f;
	}

}
