#include "stdafx.h"
#include "CircleScaleTransitionEffect.h"

#include "engine/Runtime/GraphicsCore.h"
#include "engine/Assets/Texture/TextureManager.h"
#include "engine/Functions/ECS/Component/Asset/SpriteComponent.h"
#include "engine/Functions/ECS/Component/Common/Transform2DComponent.h"

namespace NoEngine {
namespace Scene {
void CircleScaleTransitionEffect::CreateOverlay(ECS::Registry& registry, float initialAlpha, float initialScale) {
	auto winSize = GraphicsCore::sWindowManager.GetMainWindow()->GetWindowSize();
	float winW = static_cast<float>(winSize.clientWidth);
	float winH = static_cast<float>(winSize.clientHeight);

	float maxSide = std::max(winW, winH);
	float coverSize = maxSide * 3.f;

	overlayEntity_ = registry.GenerateEntity();

	auto* t2d = registry.AddComponent<Component::Transform2DComponent>(overlayEntity_);
	t2d->scale = { coverSize * initialScale, coverSize * initialScale };
	t2d->translate = { winW * 0.5f, winH * 0.5f };

	auto* sprite = registry.AddComponent<Component::SpriteComponent>(overlayEntity_);
	sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/engine/circle_soft.png");
	sprite->layer = 100000;
	sprite->color = { 0.f, 0.f, 0.f, std::clamp(initialAlpha, 0.0f, 1.0f) };
	sprite->space = SpriteSpace::Screen;
}

void CircleScaleTransitionEffect::UpdateOverlay(ECS::Registry& registry, float alpha, float scale) {
	if (overlayEntity_ == ECS::INVALID_ENTITY) return;
	if (!registry.Has<Component::SpriteComponent>(overlayEntity_)) return;
	if (!registry.Has<Component::Transform2DComponent>(overlayEntity_)) return;

	auto winSize = GraphicsCore::sWindowManager.GetMainWindow()->GetWindowSize();
	float winW = static_cast<float>(winSize.clientWidth);
	float winH = static_cast<float>(winSize.clientHeight);

	float maxSide = std::max(winW, winH);
	float coverSize = maxSide * 3.0f;

	auto* sp = registry.GetComponent<Component::SpriteComponent>(overlayEntity_);
	auto* t2d = registry.GetComponent<Component::Transform2DComponent>(overlayEntity_);

	sp->color.a = std::clamp(alpha, 0.0f, 1.0f);

	float s = std::max(scale, 0.0f);
	t2d->scale = { coverSize * s, coverSize * s };
	t2d->translate = { winW * 0.5f, winH * 0.5f };
}
}
}