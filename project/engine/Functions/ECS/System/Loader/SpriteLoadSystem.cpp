#include "stdafx.h"
#include "SpriteLoadSystem.h"
#include "../../Component/Asset/SpriteComponent.h"
#include "../../Component/Common/Transform2DComponent.h"
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
					if (registry.Has<Component::Transform2DComponent>(e)) {
						
						if (sprite->matchScaleToTexture) {
							registry.GetComponent<Component::Transform2DComponent>(e)->scale =
								Math::Vector2(static_cast<float>(sprite->textureHandle.GetWidth()),
									static_cast<float>(sprite->textureHandle.GetHeight()));
							sprite->matchScaleToTexture = false;
						}
					}
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
#include "../SystemManager.h"
REGISTER_SYSTEM(NoEngine::ECS::SpriteLoadSystem, "SpriteLoadSystem", "Loading")
