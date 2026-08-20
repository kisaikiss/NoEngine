#include "stdafx.h"
#include "LevelUpTextSystem.h"
#include "../../Component/UI/UserInterfaceComponent.h"

REFLECT_STRUCT_BEGIN(CloseLevelUpUITag, "ApplicationTag")
REFLECT_STRUCT_END(CloseLevelUpUITag)

namespace {
// ヒント枠の右アンカー基準の表示/非表示オフセット。
// anchor=(1, 0.5)(右端・縦中央)からのオフセットとして扱うことで、
// ウィンドウサイズが変わっても常に右端から一定距離の位置に表示できるようにする。
constexpr No::Vector2 kFrameAnchor = No::Vector2(1.f, 0.5f);
constexpr No::Vector2 kHiddenOffset = No::Vector2(220.f, 0.f);  // 右側の画面外
constexpr No::Vector2 kShownOffset = No::Vector2(-240.f, 0.f); // 表示位置(右端から240px)
}

void LevelUpTextSystem::Update(No::Registry& registry, float deltaTime) {
	bool isClose = true;
	No::Vector2 uiPos = No::Vector2::ZERO;
	for (auto e : registry.View<No::SpriteComponent, No::Transform2DComponent, LevelUpTextComponent, LevelUpFrameTag>()) {
		auto* transform = registry.GetComponent<No::Transform2DComponent>(e);
		auto* levelUI = registry.GetComponent<LevelUpTextComponent>(e);
		auto* sprite = registry.GetComponent<No::SpriteComponent>(e);

		// ウィンドウサイズが変わっても右端基準で表示されるよう、Screen空間+右アンカーに固定する
		sprite->space = NoEngine::SpriteSpace::Screen;
		sprite->anchor = kFrameAnchor;

		if (levelUI->isClose) {
			if (levelUI->t >= 0.f) {
				levelUI->t -= deltaTime;
			} else {
				sprite->isVisible = false;
				levelUI->isClose = false;
				levelUI->closeTimer = 0.0f;
				registry.RemoveComponent<LevelUpFrameTag>(e);

				// 表示していたヒントアイコンを破棄する
				for (auto hintEntity : registry.View<No::Transform2DComponent, No::SpriteComponent, LevelUpTextParentTag>()) {
					registry.DestroyEntity(hintEntity);
				}

				// キューに次のヒントが積まれていれば、右へはけた直後に続けて表示を開始する
				if (auto* queue = registry.GetComponent<LevelUpHintQueueComponent>(e)) {
					if (!queue->pendingTextureNames.empty()) {
						std::string nextTexture = queue->pendingTextureNames.front();
						queue->pendingTextureNames.pop_front();
						CreateLevelUpHintEntity(registry, nextTexture);
						registry.AddComponent<LevelUpFrameTag>(e);
					}
				}
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

		transform->translate = No::Lerp(kHiddenOffset, kShownOffset, No::ApplyEasing(No::EasingType::EaseInOutSine, levelUI->t));
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

	// ヒントアイコンの位置をヒント枠本体に合わせる
	for (auto e : registry.View<No::Transform2DComponent, No::SpriteComponent, LevelUpTextParentTag>()) {
		registry.GetComponent<No::Transform2DComponent>(e)->translate = uiPos;
	}
}