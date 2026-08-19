#include "stdafx.h"
#include "FadeTransitionEffect.h"

#include "engine/Runtime/GraphicsCore.h"
#include "engine/Assets/Texture/TextureManager.h"
#include "engine/Functions/ECS/Component/Asset/SpriteComponent.h"
#include "engine/Functions/ECS/Component/Common/Transform2DComponent.h"


namespace NoEngine {
namespace Scene {
void FadeTransitionEffect::CreateOverlay(ECS::Registry& registry, float initialAlpha) {
	auto winSize = GraphicsCore::sWindowManager.GetMainWindow()->GetWindowSize();
	float winW = static_cast<float>(winSize.clientWidth);
	float winH = static_cast<float>(winSize.clientHeight);

	overlayEntity_ = registry.GenerateEntity();

	auto* t2d = registry.AddComponent<Component::Transform2DComponent>(overlayEntity_);
	t2d->scale = { winW, winH };
	t2d->translate = { winW * 0.5f, winH * 0.5f };

	auto* sprite = registry.AddComponent<Component::SpriteComponent>(overlayEntity_);
	sprite->textureHandle = NoEngine::TextureManager::LoadCovertTexture("resources/engine/white1x1.png");
	sprite->layer = 100000;
	sprite->color = { 0.f, 0.f, 0.f, std::clamp(initialAlpha, 0.0f, 1.0f) };
	sprite->space = SpriteSpace::Screen;
}

void FadeTransitionEffect::UpdateOverlay(ECS::Registry& registry, float alpha) {
	if (overlayEntity_ == ECS::INVALID_ENTITY) return;
	if (!registry.Has<Component::SpriteComponent>(overlayEntity_)) return;
	if (!registry.Has<Component::Transform2DComponent>(overlayEntity_)) return;

	auto winSize = GraphicsCore::sWindowManager.GetMainWindow()->GetWindowSize();
	float winW = static_cast<float>(winSize.clientWidth);
	float winH = static_cast<float>(winSize.clientHeight);

	
	auto* sp = registry.GetComponent<Component::SpriteComponent>(overlayEntity_);
	auto* t2d = registry.GetComponent<Component::Transform2DComponent>(overlayEntity_);

	sp->color.a = std::clamp(alpha, 0.0f, 1.0f);
	t2d->scale = { winW, winH };
	t2d->translate = { winW * 0.5f, winH * 0.5f };
}
}
}