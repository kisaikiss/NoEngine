#pragma once
#include "engine/Assets/Texture/TextureManager.h"
#include "engine/NoEngine.h"
#include <string>
#include <unordered_map>

struct GameModelResource {
	std::string assetName;
	std::string filePath;
};

/// <summary>
/// ゲーム全体で共有する描画リソース。
/// テクスチャ/モデルの読み込み先を一か所にまとめる
/// </summary>
struct GameResourceComponent {
	std::unordered_map<std::string, NoEngine::TextureRef> textures;
	std::unordered_map<std::string, std::string> textureFilePaths;
	std::unordered_map<std::string, GameModelResource> models;
};

namespace GameResourceKey {
	// ---- Texture -----------------------------------------------------------
	static const char* kWhiteTexture = "WhiteTexture";
	static const char* kPlayerSprite = "PlayerSprite";
	static const char* kFieldBlock = "FieldBlock01";
	static const char* kFieldBlock02 = "FieldBlock02";
	static const char* kBackgroundBlock = "BackgroundBlock01";
	static const char* kBackgroundBlock02 = "BackgroundBlock02";


	// ---- Audio : BGM -------------------------------------------------------
	static const char* kBGMInGame = "BGMInGame";
	static const char* kBGMTitle = "BGMTitle";

	// ---- Audio : SE システム -----------------------------------------------
	static const char* kSESystemDecision = "SESystemDecision";
	static const char* kSESystemMoveCursor = "SESystemMoveCursor";
	static const char* kSESystemOpen = "SESystemOpen";

}

inline const NoEngine::TextureRef* FindGameTexture(const GameResourceComponent& resources, const std::string& key) {
	auto it = resources.textures.find(key);
	if (it == resources.textures.end()) {
		return nullptr;
	}
	return &it->second;
}

inline const NoEngine::TextureRef& GetGameTextureOrWhite(const GameResourceComponent& resources, const std::string& key) {
	const NoEngine::TextureRef* found = FindGameTexture(resources, key);
	if (found) {
		return *found;
	}
	const NoEngine::TextureRef* white = FindGameTexture(resources, GameResourceKey::kWhiteTexture);
	if (white) {
		return *white;
	}
	static NoEngine::TextureRef emptyTexture;
	return emptyTexture;
}

inline const GameModelResource* FindGameModel(const GameResourceComponent& resources, const std::string& key) {
	auto it = resources.models.find(key);
	if (it == resources.models.end()) {
		return nullptr;
	}
	return &it->second;
}

inline void InitializeGameResources(GameResourceComponent& resources) {
	resources.textures.clear();
	resources.textureFilePaths.clear();
	resources.models.clear();

	auto addTexture = [&resources](const std::string& key, const std::string& path) {
		resources.textureFilePaths[key] = path;
		resources.textures[key] = NoEngine::TextureManager::LoadCovertTexture(path);
		};
	auto addModel = [&resources](const std::string& key, const std::string& assetName, const std::string& path) {
		resources.models[key] = { assetName, path };
		No::ModelLoader::LoadModel(assetName, path);
		};

	
	addTexture(GameResourceKey::kWhiteTexture, "resources/engine/white1x1.png");
	addTexture(GameResourceKey::kPlayerSprite, "resources/engine/white1x1.png");
	//フィールドタイル
	addTexture(GameResourceKey::kFieldBlock, "resources/game/TD3/Sprite/basicAutotile.png");
	addTexture(GameResourceKey::kFieldBlock02, "resources/game/TD3/Sprite/basicAutotile02.png");
	//背景タイル
	addTexture(GameResourceKey::kBackgroundBlock, "resources/game/TD3/Sprite/wallTileset.png");
	addTexture(GameResourceKey::kBackgroundBlock02, "resources/game/TD3/Sprite/basicAutotile.png");

}
