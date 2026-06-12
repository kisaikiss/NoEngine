#include "SpriteAnimationSystem.h"
#include "../Component/Asset/Animator2DComponent.h"
#include "../Component/Asset/SpriteComponent.h"

namespace NoEngine {
namespace ECS {
void SpriteAnimationSystem::Update(Registry& registry, float deltaTime) {
	using namespace Component;
	auto view = registry.View<Animator2DComponent, SpriteComponent>();
	for (auto entity : view) {
		// コンポーネントの取得
		auto* sprite = registry.GetComponent<SpriteComponent>(entity);
		auto* animator = registry.GetComponent<Animator2DComponent>(entity);
		if (animator->stopAnimation) continue;

		animator->isAnimationEnd = false;

		// 画像サイズの取得
		uint32_t width = sprite->textureHandle.GetWidth();
		uint32_t height = sprite->textureHandle.GetHeight();

		// アニメ一コマごとのサイズと画像サイズからuvのwidth, heightを計算
		sprite->uv.width = animator->animeFrameWidth / static_cast<float>(width);
		sprite->uv.height = animator->animeFrameHeight / static_cast<float>(height);

		sprite->uv.y = static_cast<float>(animator->currentAnimation) * (animator->animeFrameHeight / static_cast<float>(height));

		// アニメーションタイマーに前frameからの経過時間を加算
		animator->timer += deltaTime;

		// タイマーが事前に設定した一コマごとの時間を超えたらコマをかえる
		if (animator->frameByFrameTime < animator->timer) {
			animator->timer = 0.f;
			sprite->uv.x += animator->animeFrameWidth / static_cast<float>(width);
			// もしアニメーションが終わりにたどり着いたら最初に戻す
			if (sprite->uv.x > static_cast<float>(animator->framesNum - 1) * (animator->animeFrameWidth / static_cast<float>(width)) ||
				sprite->uv.x > 1.f) {
				sprite->uv.x = 0.f;
				animator->isAnimationEnd = true;
			}
		} 

	}
}
}
}