#include "stdafx.h"
#include "RabbitdokuItemGetSystem.h"
#include "../../Component/RabbitdokuComponent.h"
#include "../../Game/RabbitdokuCollisionLayer.h"
#include "../../Game/RabbitdokuTag.h"
#include "application/RabbitdokuOdyssey3Plus/Game/RabbitdokuSerializer.h"
#include "../../Game/RabbitdokuResetEvent.h"

void RabbitItemGetSystem::Update(No::Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	auto view = registry.PollAllEvents<RabbitdokuItemGetEvent>();

	for (const auto& event : view) {
		auto* player = registry.GetComponent<Rabbitdoku>(event.player);
		if (registry.GetComponent<SaveTag>(event.item)) {
			if (No::InputIsTrigger("Save")) {
				auto* data = registry.GetComponent<SaveData>(event.player);
				RabbitdokuSerializer::GameSave(registry,
					registry.GetComponent<No::Transform2DComponent>(event.player)->translate,
					data->death, data->totalDeath);
				data->respawnPoint = registry.GetComponent<No::Transform2DComponent>(event.player)->translate;
				GenerateLight(registry, event.player);
				for (uint32_t i = 0; i < 3; i++) {
					GenerateStars(registry, event.player);
				}
			}
		}

		if (registry.GetComponent<EnemyTag>(event.item)) {
			player->state = RabbitdokuState::Dead;
		}

		if (registry.GetComponent<SpringComponent>(event.item)) {
			player->yVelocity = -registry.GetComponent<SpringComponent>(event.item)->force;
			player->canDoubleJump = true;
			registry.GetComponent<No::Animator2DComponent>(event.item)->framesNum = 5;
		}

		if (auto* block = registry.GetComponent<CollapseBlockComponent>(event.item)) {
			block->startCollapse = true;
			RabbitdokuPushBackEvent e;
			e.player = event.player;
			e.position = event.position;
			registry.EmitEvent(e);
		}
	}

}

void RabbitItemGetSystem::GenerateLight(No::Registry& registry, No::Entity playerEntity) {
	auto* playerTransform = registry.GetComponent<No::Transform2DComponent>(playerEntity);
	auto e = registry.GenerateEntity();
	auto* t = registry.AddComponent<No::Transform2DComponent>(e);
	t->translate = playerTransform->translate;
	t->translate.y -= 64.f;
	t->scale = 64.f;
	t->rotation = 0.0f;
	auto* s = registry.AddComponent<No::SpriteComponent>(e);
	s->textureFilePath = "resources/game/RabbitdokuOdyssey3Plus/Sprite/Light.png";
	s->layer = 19;
	auto* a = registry.AddComponent<No::Animator2DComponent>(e);
	a->animeFrameHeight = 64.f;
	a->animeFrameWidth = 64.f;
	a->framesNum = 5;
	a->frameByFrameTime = 0.1f;
	registry.AddComponent<SmokeEffectTag>(e);
	registry.AddComponent<No::Velocity2DComponent>(e)->linear.y = -32.f;
}

void RabbitItemGetSystem::GenerateStars(No::Registry& registry, No::Entity playerEntity) {
	auto* playerTransform = registry.GetComponent<No::Transform2DComponent>(playerEntity);
	auto e = registry.GenerateEntity();
	auto* t = registry.AddComponent<No::Transform2DComponent>(e);
	t->translate = playerTransform->translate;
	t->translate += No::GetRandomVal(No::Vector2(-128.f, -128.f), No::Vector2(128.f, -32.f));
	t->scale = 64.f;
	t->rotation = 0.0f;
	auto* s = registry.AddComponent<No::SpriteComponent>(e);
	s->textureFilePath = "resources/game/RabbitdokuOdyssey3Plus/Sprite/Stars.png";
	s->layer = 19;
	auto* a = registry.AddComponent<No::Animator2DComponent>(e);
	a->animeFrameHeight = 64.f;
	a->animeFrameWidth = 64.f;
	a->framesNum = 6;
	a->frameByFrameTime = 0.1f;
	registry.AddComponent<SmokeEffectTag>(e);
	registry.AddComponent<No::Velocity2DComponent>(e)->linear.y = 64.f;
}
