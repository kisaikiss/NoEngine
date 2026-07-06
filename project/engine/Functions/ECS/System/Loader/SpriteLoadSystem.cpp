#include "stdafx.h"
#include "SpriteLoadSystem.h"
#include "../../Component/Asset/SpriteComponent.h"
#include "engine/Assets/Texture/TextureManager.h"
#include "engine/Assets/AssetManager.h"

namespace NoEngine {
namespace ECS {
void SpriteLoadSystem::Update(Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	auto view = registry.View<Component::SpriteComponent>();
	for (auto e : view) {
		auto* sprite = registry.GetComponent<Component::SpriteComponent>(e);
		// 通常テクスチャ
		if (!sprite->textureHandle.IsValid()) {
			auto texture = TextureManager::LoadCovertTexture(AssetManager::GetFilePathFromAddressableName(sprite->textureName));

			// 正常に読み込まれたなら
			if (texture.IsValid()) {
				// 実際のスプライトへ代入する
				sprite->textureHandle = texture;
			}
		}
		
		// マスク用テクスチャ	
		if (!sprite->maskTextureHandle.IsValid() && sprite->useMask) {
			auto texture = TextureManager::LoadCovertTexture(AssetManager::GetFilePathFromAddressableName(sprite->maskTextureName));

			// 正常に読み込まれたなら
			if (texture.IsValid()) {
				// 実際のスプライトへ代入する
				sprite->maskTextureHandle = texture;
			}
		}
	}

}
}
}