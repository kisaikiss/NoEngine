#include "stdafx.h"
#include "FontLoadSystem.h"
#include "engine/Functions/ECS/Component/Asset/TextComponent.h"
#include "engine/Assets/AssetManager.h"

namespace NoEngine::ECS {
void FontLoadSystem::Update(Registry& registry, float deltaTime) {
	static_cast<void>(deltaTime);
	const uint64_t generation = AssetManager::GetAddressableGeneration();
	for (auto entity : registry.View<Component::TextComponent>()) {
		auto* text = registry.GetComponent<Component::TextComponent>(entity);
		if (text->loadedFontGeneration == generation && text->loadedFontName == text->fontName) continue;

		text->loadedFontGeneration = generation;
		text->loadedFontName = text->fontName;
		text->fontHandle = nullptr;
		if (text->fontName.empty()) continue;

		const std::string path = AssetManager::GetFilePathFromAddressableName(text->fontName);
		if (!path.empty()) text->fontHandle = FontManager::LoadFontFile(path);
	}
}
}

#include "../SystemManager.h"
REGISTER_SYSTEM(NoEngine::ECS::FontLoadSystem, "FontLoadSystem", "Loading")
