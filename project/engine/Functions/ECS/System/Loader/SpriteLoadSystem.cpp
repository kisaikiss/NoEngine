#include "stdafx.h"
#include "SpriteLoadSystem.h"
#include "../../Component/SpriteComponent.h"
#include "engine/Assets/Texture/TextureManager.h"

namespace NoEngine {
namespace ECS {
void SpriteLoadSystem::Update(Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	auto view = registry.View<Component::SpriteComponent>();
	for (auto e : view) {
		auto* sprite = registry.GetComponent<Component::SpriteComponent>(e);
		if (!sprite->textureHandle.IsValid()) {
			// すでに同じテクスチャが読み込まれているなら再読み込みされないようにLoadConvertTexture()は作成されている。
			auto texture = TextureManager::LoadCovertTexture(sprite->textureFilePath);

			// 正常に読み込まれたなら
			if (texture.IsValid()) {
				// 実際のスプライトへ代入する
				sprite->textureHandle = texture;
			}
		}
		

	}

}
}
}