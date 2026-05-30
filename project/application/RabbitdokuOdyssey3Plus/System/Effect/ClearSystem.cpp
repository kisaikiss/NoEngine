#include "stdafx.h"
#include "ClearSystem.h"

#include "../../Game/RabbitdokuTag.h"
#include "../../Game/RabbitdokuSerializer.h"

void ClearSystem::Update(No::Registry& registry, float deltaTime) {
	(void)registry;
	(void)deltaTime;
	auto view = registry.View<ClearTag,No::Transform2DComponent, No::Animator2DComponent, No::SpriteComponent>();
	for (auto e : view) {
		auto* a = registry.GetComponent<No::Animator2DComponent>(e);
		auto* c = registry.GetComponent<ClearTag>(e);
		if (c->isAnimationEnd) {
			auto cameraView = registry.View<No::ActiveCamera2DTag, No::Transform2DComponent>();
			No::Transform2DComponent* cameraT = nullptr;
			for (auto ce : cameraView) {
				cameraT = registry.GetComponent<No::Transform2DComponent>(ce);
			}
			auto* t = registry.GetComponent<No::Transform2DComponent>(e);
			t->translate.x = No::EaseInOutBack(0.0f, 2000.f, c->t);
			c->t += deltaTime * 0.2f;
			if (c->t > 0.7f) {
				SceneTransitionInEvent change;
				change.stageName = "Title";
				change.saveData = RabbitdokuSerializer::DeleteSave(registry);
				registry.EmitEvent(change);
			}

			if (!cameraT) {
				t->parent = nullptr;
			} else {
				t->parent = cameraT;
			}
		}

		if (a->isAnimationEnd) {
			a->stopAnimation = true;
			a->isAnimationEnd = false;
			auto* s = registry.GetComponent<No::SpriteComponent>(e);
			s->uv.x = 12.f / 13.f;
			
			c->isAnimationEnd = true;
			for (uint32_t i = 0; i < 100; i++) {
				GenerateEffect(registry);
			}
		}


	}
}

void ClearSystem::GenerateEffect(No::Registry& registry) {
	auto view = registry.View<No::ActiveCamera2DTag, No::Transform2DComponent>();
	No::Transform2DComponent* cameraT = nullptr;
	for (auto e : view) {
		cameraT = registry.GetComponent<No::Transform2DComponent>(e);
	}
	if (!cameraT)return;
	auto e = registry.GenerateEntity();
	auto* t = registry.AddComponent<No::Transform2DComponent>(e);
	t->translate = cameraT->translate;
	t->scale = 64.f;
	t->rotation = 0.0f;
	auto* s = registry.AddComponent<No::SpriteComponent>(e);
	s->textureFilePath = No::GetRandomValNormalized() > 0.f ? "resources/game/RabbitdokuOdyssey3Plus/Sprite/CommonEffect01.png" : "resources/game/RabbitdokuOdyssey3Plus/Sprite/CommonEffect02.png";
	s->layer = 24;
	auto* a = registry.AddComponent<No::Animator2DComponent>(e);
	a->animeFrameHeight = 64.f;
	a->animeFrameWidth = 64.f;
	a->framesNum = 5;
	a->frameByFrameTime = 0.4f;
	registry.AddComponent<SmokeEffectTag>(e);
	registry.AddComponent<No::Velocity2DComponent>(e)->linear = No::GetRandomVal(No::Vector2(-800.f, -800.f), No::Vector2(800.f, 800.f));

}
