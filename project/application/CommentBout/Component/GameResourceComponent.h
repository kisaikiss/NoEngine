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

namespace CommentBoutResourceKey {
	// ---- Texture -----------------------------------------------------------
	static const char* kWhiteTexture = "WhiteTexture";
	static const char* kPlayerSprite = "PlayerSprite";
	static const char* kPlayerAttackSprite = "PlayerAttackSprite";
	static const char* kPauseTitle = "PauseTitle";
	static const char* kPauseToGame = "PauseToGame";
	static const char* kRestart = "Restart";
	static const char* kOptionMenu = "OptionMenu";
	static const char* kPauseToTitle = "PauseToTitle";
	static const char* kTitleLogo = "TitleLogo";
	static const char* kGameStart = "GameStart";
	static const char* kGameEnd = "GameEnd";
	static const char* kMaster = "Master";
	static const char* kBGM = "BGM";
	static const char* kSE = "SE";
	static const char* kVibration = "Vibration";
	static const char* kBack = "Back";
	static const char* kOn = "On";
	static const char* kOff = "Off";
	static const char* kRewardOrbSprite = "RewardOrbSprite";
	static const char* kRailProgressStart = "RailProgressStart";
	static const char* kRailProgressGoal = "RailProgressGoal";
	static const char* kRailProgressBarBase = "RailProgressBarBase";
	static const char* kRailProgressBarFill = "RailProgressBarFill";
	static const char* kRailProgressPlayerMarker = "RailProgressPlayerMarker";

	//吹き出し
	static const char* kAttackOrbEffectSmall = "AttackOrbEffectSmall";
	static const char* kAttackOrbEffectMedium = "AttackOrbEffectMedium";
	static const char* kAttackOrbEffectBig = "AttackOrbEffectBig";


	// ---- Model -------------------------------------------------------------
	static const char* kEnemyModel = "EnemyModel";
	static const char* kShootEnemyModel = "ShootEnemyModel";
	static const char* kBossModel = "BossModel";
	static const char* kEnemyBulletModel = "EnemyBulletModel";
	static const char* kGrassModel = "GrassModel";
	static const char* kGroundModel = "GroundModel";
	static const char* kBuildingModel = "BuildingModel";
	static const char* kSkydomeModel = "SkydomeModel";
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
	const NoEngine::TextureRef* white = FindGameTexture(resources, CommentBoutResourceKey::kWhiteTexture);
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

inline void InitializeCommentBoutGameResources(GameResourceComponent& resources) {
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

	// ---- Texture path catalog --------------------------------------------
	addTexture(CommentBoutResourceKey::kWhiteTexture, "resources/engine/white1x1.png");
	addTexture(CommentBoutResourceKey::kPlayerSprite, "resources/engine/white1x1.png");
	addTexture(CommentBoutResourceKey::kPlayerAttackSprite, "resources/engine/white1x1.png");
	addTexture(CommentBoutResourceKey::kPauseTitle, "resources/game/td_3105/Sprite/Pause.png");
	addTexture(CommentBoutResourceKey::kPauseToGame, "resources/game/td_3105/Sprite/PauseToGame.png");
	addTexture(CommentBoutResourceKey::kRestart, "resources/game/td_3105/Sprite/Restart.png");
	addTexture(CommentBoutResourceKey::kOptionMenu, "resources/game/td_3105/Sprite/OptionMenu.png");
	addTexture(CommentBoutResourceKey::kPauseToTitle, "resources/game/td_3105/Sprite/PauseToTitle.png");
	addTexture(CommentBoutResourceKey::kTitleLogo, "resources/game/td_3105/Sprite/TitleRogo.png");
	addTexture(CommentBoutResourceKey::kGameStart, "resources/game/td_3105/Sprite/GameStart.png");
	addTexture(CommentBoutResourceKey::kGameEnd, "resources/game/td_3105/Sprite/GameEnd.png");
	addTexture(CommentBoutResourceKey::kMaster, "resources/game/td_3105/Sprite/Master.png");
	addTexture(CommentBoutResourceKey::kBGM, "resources/game/td_3105/Sprite/BGM.png");
	addTexture(CommentBoutResourceKey::kSE, "resources/game/td_3105/Sprite/SE.png");
	addTexture(CommentBoutResourceKey::kVibration, "resources/game/td_3105/Sprite/Vibration.png");
	addTexture(CommentBoutResourceKey::kBack, "resources/game/td_3105/Sprite/Back.png");
	addTexture(CommentBoutResourceKey::kOn, "resources/game/td_3105/Sprite/On.png");
	addTexture(CommentBoutResourceKey::kOff, "resources/game/td_3105/Sprite/Off.png");
	addTexture(CommentBoutResourceKey::kRewardOrbSprite, "resources/engine/white1x1.png");
	addTexture(CommentBoutResourceKey::kRailProgressStart, "resources/engine/white1x1.png");
	addTexture(CommentBoutResourceKey::kRailProgressGoal, "resources/engine/white1x1.png");
	addTexture(CommentBoutResourceKey::kRailProgressBarBase, "resources/engine/white1x1.png");
	addTexture(CommentBoutResourceKey::kRailProgressBarFill, "resources/engine/white1x1.png");
	addTexture(CommentBoutResourceKey::kRailProgressPlayerMarker, "resources/engine/white1x1.png");
	//吹き出し
	addTexture(CommentBoutResourceKey::kAttackOrbEffectSmall, "resources/game/td_3105/Sprite/OH.png");
	addTexture(CommentBoutResourceKey::kAttackOrbEffectMedium, "resources/game/td_3105/Sprite/BOOM.png");
	addTexture(CommentBoutResourceKey::kAttackOrbEffectBig, "resources/game/td_3105/Sprite/POW.png");

	// ---- Model path catalog ----------------------------------------------
	addModel(CommentBoutResourceKey::kEnemyModel, "commentbout_enemy_model", "resources/game/td_3105/Model/Enemy/Enemy.obj");
	addModel(CommentBoutResourceKey::kShootEnemyModel, "commentbout_shoot_enemy_model", "resources/game/td_3105/Model/cube/cube.obj");
	addModel(CommentBoutResourceKey::kBossModel, "commentbout_boss_model", "resources/game/td_3105/Model/cube/cube.obj");
	addModel(CommentBoutResourceKey::kEnemyBulletModel, "commentbout_enemy_bullet_model", "resources/game/td_3105/Model/Bullet/Bullet.obj");
	addModel(CommentBoutResourceKey::kGrassModel, "commentbout_grass_model", "resources/game/td_3105/Model/cube/cube.obj");
	addModel(CommentBoutResourceKey::kGroundModel, "commentbout_ground_model", "resources/game/td_3105/Model/cube/cube.obj");
	//	addModel(CommentBoutResourceKey::kGroundModel, "commentbout_ground_model", "resources/game/td_3105/Model/Field/Field.obj");
	addModel(CommentBoutResourceKey::kBuildingModel, "commentbout_building_model", "resources/game/td_3105/Model/cube/cube.obj");
	addModel(CommentBoutResourceKey::kSkydomeModel, "commentbout_skydome_model", "resources/game/td_3105/Model/skydome/skydome.gltf");
}
