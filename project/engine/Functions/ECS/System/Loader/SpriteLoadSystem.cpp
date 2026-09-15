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
	const uint64_t currentGeneration = AssetManager::GetAddressableGeneration();

	for (auto e : view) {
		auto* sprite = registry.GetComponent<Component::SpriteComponent>(e);
		const bool tableChanged = (sprite->loadedGeneration != currentGeneration);

		// 通常テクスチャ
		if (tableChanged || sprite->loadedTextureName != sprite->textureName) {
			sprite->loadedTextureName = sprite->textureName;

			std::string path = AssetManager::GetFilePathFromAddressableName(sprite->textureName);
			if (!path.empty()) {
				auto texture = TextureManager::LoadCovertTexture(path);
				if (texture.IsValid()) {
					sprite->textureHandle = texture;
				}
			}
		}

		// マスク用テクスチャ
		if (sprite->useMask && (tableChanged || sprite->loadedMaskTextureName != sprite->maskTextureName)) {
			sprite->loadedMaskTextureName = sprite->maskTextureName;

			std::string maskPath = AssetManager::GetFilePathFromAddressableName(sprite->maskTextureName);
			if (!maskPath.empty()) {
				auto texture = TextureManager::LoadCovertTexture(maskPath);
				if (texture.IsValid()) {
					sprite->maskTextureHandle = texture;
				}
			}
		}

		sprite->loadedGeneration = currentGeneration;
	}
}
}
}