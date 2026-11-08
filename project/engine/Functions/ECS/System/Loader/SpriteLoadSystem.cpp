#include "stdafx.h"
#include "SpriteLoadSystem.h"
#include "../../Component/Asset/SpriteComponent.h"
#include "../../Component/Common/Transform2DComponent.h"
#include "engine/Assets/Texture/TextureManager.h"
#include "engine/Assets/AssetManager.h"

namespace NoEngine {
namespace ECS {
void SpriteLoadSystem::Update(Registry& registry, float deltaTime) {
	auto view = registry.View<Component::SpriteComponent>();
	const uint64_t currentGeneration = AssetManager::GetAddressableGeneration();

	for (auto e : view) {
		auto* sprite = registry.GetComponent<Component::SpriteComponent>(e);
		const bool tableChanged = (sprite->loadedGeneration != currentGeneration);

		bool assignedTexture = false;
		// Addressableから画像を割り当て、GIFなら再生を開始する。
		if (tableChanged || sprite->loadedTextureName != sprite->textureName) {
			sprite->loadedTextureName = sprite->textureName;

			const auto path = sprite->textureName.empty() ? std::string{} : AssetManager::GetFilePathFromAddressableName(sprite->textureName);
			if (!path.empty()) {
				assignedTexture = sprite->SetTextureFromPath(path);
				sprite->gifFromAddressable = sprite->gifTexture.IsLoaded();
				if (assignedTexture && sprite->matchScaleToTexture) {
					if (auto* transform = registry.GetComponent<Component::Transform2DComponent>(e)) {
						transform->scale = { static_cast<float>(sprite->textureHandle.GetWidth()),
							static_cast<float>(sprite->textureHandle.GetHeight()) };
						sprite->matchScaleToTexture = false;
					}
				}
			} else if (sprite->gifFromAddressable) {
				sprite->gifFromAddressable = false;
				sprite->gifTexture = {};
				sprite->textureHandle = nullptr;
			}
		}
		if (!assignedTexture && sprite->gifTexture.IsLoaded()) {
			sprite->gifTexture.Advance(deltaTime);
			sprite->textureHandle = sprite->gifTexture.GetFrame();
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
