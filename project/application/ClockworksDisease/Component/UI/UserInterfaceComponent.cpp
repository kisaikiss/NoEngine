#include "UserInterfaceComponent.h"
#include "engine/NoEngine.h"

REFLECT_STRUCT_BEGIN(StaminaGaugeComponent, "Application")
REFLECT_FIELD(disappearanceTime)
REFLECT_STRUCT_END(StaminaGaugeComponent)


REFLECT_STRUCT_BEGIN(LevelUpTextComponent, "Application")
REFLECT_FIELD(isClose),
REFLECT_FIELD(closeTime)
REFLECT_STRUCT_END(LevelUpTextComponent)


REFLECT_STRUCT_BEGIN(StaminaGaugeParentTag, "ApplicationTag")
REFLECT_STRUCT_END(StaminaGaugeParentTag)


REFLECT_STRUCT_BEGIN(CanMagicUITag, "ApplicationTag")
REFLECT_FIELD(isBackground)
REFLECT_STRUCT_END(CanMagicUITag)

No::Entity CreateLevelUpHintEntity(No::Registry& registry, const std::string& textureName) {
	auto textEntity = registry.GenerateEntity();
	registry.AddComponent<No::Transform2DComponent>(textEntity)->scale = No::Vector2(468.f, 720.f);
	auto* sprite = registry.AddComponent<No::SpriteComponent>(textEntity);
	sprite->textureName = textureName;
	sprite->layer = 1;
	// ウィンドウサイズが変わっても右端基準で表示されるよう、ヒント枠と同じくScreen空間+右アンカーにする
	sprite->space = NoEngine::SpriteSpace::Screen;
	sprite->anchor = No::Vector2(1.f, 0.5f);
	registry.AddComponent<LevelUpTextParentTag>(textEntity);
	return textEntity;
}