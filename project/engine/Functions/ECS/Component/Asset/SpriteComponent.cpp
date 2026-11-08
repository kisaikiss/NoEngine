#include "SpriteComponent.h"
#include "engine/Editor/Reflection/ReflectionMacros.h"
#include "engine/Utilities/Conversion/ConvertString.h"

bool NoEngine::Component::SpriteComponent::SetTextureFromPath(const std::string& path) {
	gifFromAddressable = false;
	const auto widePath = ConvertString(path);
	if (IsGifTexture(widePath)) {
		const bool loaded = gifTexture.Load(widePath);
		textureHandle = gifTexture.GetFrame();
		return loaded;
	}

	if (gifTexture.IsLoaded()) {
		gifTexture = {};
		textureHandle = nullptr;
	}
	auto texture = TextureManager::LoadCovertTexture(path);
	if (!texture.IsValid()) return false;
	textureHandle = std::move(texture);
	return true;
}

REFLECT_STRUCT_BEGIN(NoEngine::Component::SpriteComponent, "Asset")
REFLECT_ENUM_FIELD(space),
REFLECT_FIELD(anchor),
REFLECT_FIELD(isVisible),
REFLECT_FIELD(pivot),
REFLECT_FIELD(flipX),
REFLECT_FIELD(flipY),
REFLECT_FIELD(layer),
REFLECT_FIELD(orderInLayer),
REFLECT_FIELD(color),
REFLECT_FIELD(uv),
REFLECT_ASSET_FIELD(textureName, Texture),
REFLECT_FIELD(nonRenderAngle),
REFLECT_FIELD(useMask),
REFLECT_FIELD(fill),
REFLECT_ASSET_FIELD(maskTextureName, Texture)
REFLECT_STRUCT_END(NoEngine::Component::SpriteComponent)
