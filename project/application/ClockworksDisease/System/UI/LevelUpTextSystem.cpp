#include "stdafx.h"
#include "LevelUpTextSystem.h"
#include "../../Component/UI/UserInterfaceComponent.h"

REFLECT_STRUCT_BEGIN(CloseLevelUpUITag)
REFLECT_STRUCT_END(CloseLevelUpUITag)

void LevelUpTextSystem::Update(No::Registry& registry, float deltaTime) {
	bool isClose = true;
	bool isEnd = false;
	No::Vector2 uiPos = No::Vector2::ZERO;
	for (auto e : registry.View<No::SpriteComponent, No::Transform2DComponent, LevelUpTextComponent, LevelUpFrameTag>()) {
		auto* transform = registry.GetComponent<No::Transform2DComponent>(e);
		auto* levelUI = registry.GetComponent<LevelUpTextComponent>(e);
		auto* sprite = registry.GetComponent<No::SpriteComponent>(e);
		if (levelUI->isClose) {
			if (levelUI->t >= 0.f) {
				levelUI->t -= deltaTime;
			} else {
				sprite->isVisible = false;
				levelUI->isClose = false;
				isEnd = true;
				levelUI->closeTimer = 0.0f;
				registry.RemoveComponent<LevelUpFrameTag>(e);

			}
		} else {
			sprite->isVisible = true;
			if (levelUI->t <= 1.f) {
				levelUI->t += deltaTime * 0.6f;
			}
			if (No::InputIsPress("CloseLevelUpUI")) {
				levelUI->closeTimer += deltaTime;
				if (levelUI->closeTimer >= levelUI->closeTime) {
					levelUI->isClose = true;
				}
			}

		}
		
		transform->translate = No::EaseInOutSine(No::Vector2(1500.f, 360.f), No::Vector2(1040.f, 360.f), levelUI->t);
		uiPos = transform->translate;
		isClose = levelUI->isClose;
	}

	for (auto e : registry.View<No::SpriteComponent, CloseLevelUpUITag>()) {
		auto* sprite = registry.GetComponent<No::SpriteComponent>(e);
		auto* levelUI = registry.GetComponent<CloseLevelUpUITag>(e);
		if (isClose) {
			if (levelUI->t >= 0.f) {
				levelUI->t -= deltaTime;
			}
		} else {
			if (levelUI->t <= 1.f) {
				levelUI->t += deltaTime * 0.3f;
			}
		}

		sprite->color.a = std::lerp(0.0f, 1.0f, levelUI->t);
	}

	// スタミナゲージの周りの部分の位置と色をスタミナゲージ本体に合わせる
	for (auto e : registry.View < No::Transform2DComponent, No::SpriteComponent, LevelUpTextParentTag>()) {
		registry.GetComponent<No::Transform2DComponent>(e)->translate = uiPos;
		if (isEnd) {
			registry.DestroyEntity(e);
		}
	}

}
