#include "stdafx.h"
#include "ScreenTransitionEffectSystem.h"
#include "../../Game/RabbitdokuTag.h"
#include "../../Game/RabbitdokuResetEvent.h"

namespace {
const float kTransitionSpeed = 1.5f;
std::string sCurrentScene;
}

void ScreenTransitionEffectSystem::Update(No::Registry& registry, float deltaTime) {
	sCurrentScene = No::GetCurrentSceneName(registry);
	TransitionIn(registry, deltaTime);
	TransitionOut(registry, deltaTime);
}

void ScreenTransitionEffectSystem::TransitionIn(No::Registry& registry, float deltaTime) {
	No::Transform2DComponent* cameraT = nullptr;
	auto cameraView = registry.View<No::ActiveCamera2DTag, No::Transform2DComponent>();
	for (auto e : cameraView) {
		cameraT = registry.GetComponent<No::Transform2DComponent>(e);
	}

	auto event = registry.PollEvent<SceneTransitionInEvent>();
	if (event.has_value()) {
		auto e = registry.GenerateEntity();
		if (event->stageName.empty()) {
			nextScene_ = sCurrentScene;
		} else {
			nextScene_ = event->stageName;
		}
		registry.AddComponent<TransitionInTag>(e);
		auto* t = registry.AddComponent<No::Transform2DComponent>(e);
		t->scale = No::Vector2(1280.f, 848.f);
		t->translate.y = -848.f;
		t->parent = cameraT;
		auto* s = registry.AddComponent<No::SpriteComponent>(e);
		s->textureFilePath = "resources/game/RabbitdokuOdyssey3Plus/Sprite/SceneChange01.png";
		s->layer = 1000;
	}

	if (cameraT) {
		auto view = registry.View<TransitionInTag, No::SpriteComponent, No::Transform2DComponent>();
		for (auto e : view) {
			auto tag = registry.GetComponent<TransitionInTag>(e);
			tag->t += deltaTime * kTransitionSpeed;
			auto t = registry.GetComponent<No::Transform2DComponent>(e);
			t->translate = No::EaseInExpo(No::Vector2(0.0f, -848.f), No::Vector2::ZERO, tag->t);
			if (tag->t > 1.f) {
				t->parent = nullptr;
				t->translate = cameraT->translate;
				t->rotation = cameraT->rotation;
				t->scale.x = 1280.f * cameraT->scale.x;
				t->scale.y = 848.f * cameraT->scale.y;
				RabbitdokuResetEvent dead;
				dead.nextScene = nextScene_;
				if (nextScene_ != No::GetCurrentSceneName(registry)) No::SoundAllCompleteStop();
				registry.EmitEvent(dead);
			}

		}
	}
}

void ScreenTransitionEffectSystem::TransitionOut(No::Registry& registry, float deltaTime) {
	No::Transform2DComponent* cameraT = nullptr;
	auto cameraView = registry.View<No::ActiveCamera2DTag, No::Transform2DComponent>();
	for (auto e : cameraView) {
		cameraT = registry.GetComponent<No::Transform2DComponent>(e);
	}

	auto event = registry.PollEvent<SceneTransitionOutEvent>();
	if (event.has_value()) {
		auto e = registry.GenerateEntity();
		registry.AddComponent<TransitionOutTag>(e);
		auto* t = registry.AddComponent<No::Transform2DComponent>(e);
		t->scale = No::Vector2(1280.f, 848.f);
		t->parent = cameraT;
		auto* s = registry.AddComponent<No::SpriteComponent>(e);
		s->textureFilePath = "resources/game/RabbitdokuOdyssey3Plus/Sprite/SceneChange01.png";
		s->layer = 1000;
	}

	auto view = registry.View<TransitionOutTag, No::SpriteComponent, No::Transform2DComponent>();
	for (auto e : view) {
		auto tag = registry.GetComponent<TransitionOutTag>(e);
		tag->t += deltaTime * kTransitionSpeed;
		auto t = registry.GetComponent<No::Transform2DComponent>(e);
		t->translate = No::EaseInExpo(No::Vector2::ZERO, No::Vector2(0.0f, -848.f), tag->t);
		if (tag->t > 1.f) {
			registry.DestroyEntity(e);
		}

	}
}
