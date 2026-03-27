#pragma once
#include "engine/NoEngine.h"
#include "application/CommentBout/Utility/CBSpriteLayer.h"
#include "externals/nlohmann/json.hpp"
#include <fstream>

struct PlayerConfigComponent {
	// PlayerComponent
	float moveSpeed = 480.0f;
	float acceleration = 2400.0f;
	float deceleration = 3000.0f;
	float maxSpeed = 480.0f;
	float invincibleDurationDefault = 0.35f;

	// PlayerAttackComponent
	No::Vector2 attackSpawnOffset = { 0.0f, -80.0f };
	No::Vector2 attackSize = { 140.0f, 140.0f };
	float attackVisibleTime = 0.35f;
	int attackLayer = static_cast<int>(CommentBout::ToLayer(CommentBout::SpriteLayer::Effect));
	int attackPower = 10;

	// Health / Invincible
	int playerMaxHp = 10;
	float invincibleDuration = 0.35f;

	// DamageFlash (player)
	float flashDuration = 0.16f;
	bool flashBlinkEnabled = true;
	float flashBlinkHz = 14.0f;
	float flashMinAlpha = 0.35f;
	float flashMaxAlpha = 0.75f;
	No::Vector3 flashColorRGB = { 1.0f, 0.35f, 0.35f };

	// PlayerHitboxComponent
	bool useCameraGateForPlayerHit = true;
	float cameraGateNear = 0.0f;
	float cameraGateDepth = 0.5f;
	float cameraGateHalfWidth = 0.6f;
	float cameraGateHalfHeight = 0.32f;

	// StartTransform2DComponent
	No::Vector2 startPosition = { 640.0f, 600.0f };
	No::Vector2 startScale = { 128.0f, 200.0f };
	float startRotation = 0.0f;
};

inline std::string MakePlayerConfigFilePath() {
	return "resources/game/td_3105/RailData/PlayerConfig.json";
}

inline bool LoadPlayerConfig(PlayerConfigComponent& outConfig) {
	std::ifstream ifs(MakePlayerConfigFilePath());
	if (!ifs) {
		return false;
	}

	nlohmann::json j;
	ifs >> j;
	if (!j.is_object()) {
		return false;
	}

	if (j.contains("moveSpeed") && j["moveSpeed"].is_number()) outConfig.moveSpeed = j["moveSpeed"].get<float>();
	if (j.contains("acceleration") && j["acceleration"].is_number()) outConfig.acceleration = j["acceleration"].get<float>();
	if (j.contains("deceleration") && j["deceleration"].is_number()) outConfig.deceleration = j["deceleration"].get<float>();
	if (j.contains("maxSpeed") && j["maxSpeed"].is_number()) outConfig.maxSpeed = j["maxSpeed"].get<float>();
	if (j.contains("invincibleDurationDefault") && j["invincibleDurationDefault"].is_number()) outConfig.invincibleDurationDefault = j["invincibleDurationDefault"].get<float>();

	if (j.contains("attackSpawnOffset") && j["attackSpawnOffset"].is_array() && j["attackSpawnOffset"].size() >= 2) {
		outConfig.attackSpawnOffset.x = j["attackSpawnOffset"][0].get<float>();
		outConfig.attackSpawnOffset.y = j["attackSpawnOffset"][1].get<float>();
	}
	if (j.contains("attackSize") && j["attackSize"].is_array() && j["attackSize"].size() >= 2) {
		outConfig.attackSize.x = j["attackSize"][0].get<float>();
		outConfig.attackSize.y = j["attackSize"][1].get<float>();
	}
	if (j.contains("attackVisibleTime") && j["attackVisibleTime"].is_number()) outConfig.attackVisibleTime = j["attackVisibleTime"].get<float>();
	if (j.contains("attackLayer") && j["attackLayer"].is_number_integer()) outConfig.attackLayer = j["attackLayer"].get<int>();
	if (j.contains("attackPower") && j["attackPower"].is_number_integer()) outConfig.attackPower = j["attackPower"].get<int>();

	if (j.contains("playerMaxHp") && j["playerMaxHp"].is_number_integer()) outConfig.playerMaxHp = j["playerMaxHp"].get<int>();
	if (j.contains("invincibleDuration") && j["invincibleDuration"].is_number()) outConfig.invincibleDuration = j["invincibleDuration"].get<float>();

	if (j.contains("flashDuration") && j["flashDuration"].is_number()) outConfig.flashDuration = j["flashDuration"].get<float>();
	if (j.contains("flashBlinkEnabled") && j["flashBlinkEnabled"].is_boolean()) outConfig.flashBlinkEnabled = j["flashBlinkEnabled"].get<bool>();
	if (j.contains("flashBlinkHz") && j["flashBlinkHz"].is_number()) outConfig.flashBlinkHz = j["flashBlinkHz"].get<float>();
	if (j.contains("flashMinAlpha") && j["flashMinAlpha"].is_number()) outConfig.flashMinAlpha = j["flashMinAlpha"].get<float>();
	if (j.contains("flashMaxAlpha") && j["flashMaxAlpha"].is_number()) outConfig.flashMaxAlpha = j["flashMaxAlpha"].get<float>();
	if (j.contains("flashColorRGB") && j["flashColorRGB"].is_array() && j["flashColorRGB"].size() >= 3) {
		outConfig.flashColorRGB.x = j["flashColorRGB"][0].get<float>();
		outConfig.flashColorRGB.y = j["flashColorRGB"][1].get<float>();
		outConfig.flashColorRGB.z = j["flashColorRGB"][2].get<float>();
	}

	if (j.contains("useCameraGateForPlayerHit") && j["useCameraGateForPlayerHit"].is_boolean()) outConfig.useCameraGateForPlayerHit = j["useCameraGateForPlayerHit"].get<bool>();
	if (j.contains("cameraGateNear") && j["cameraGateNear"].is_number()) outConfig.cameraGateNear = j["cameraGateNear"].get<float>();
	if (j.contains("cameraGateDepth") && j["cameraGateDepth"].is_number()) outConfig.cameraGateDepth = j["cameraGateDepth"].get<float>();
	if (j.contains("cameraGateHalfWidth") && j["cameraGateHalfWidth"].is_number()) outConfig.cameraGateHalfWidth = j["cameraGateHalfWidth"].get<float>();
	if (j.contains("cameraGateHalfHeight") && j["cameraGateHalfHeight"].is_number()) outConfig.cameraGateHalfHeight = j["cameraGateHalfHeight"].get<float>();

	if (j.contains("startPosition") && j["startPosition"].is_array() && j["startPosition"].size() >= 2) {
		outConfig.startPosition.x = j["startPosition"][0].get<float>();
		outConfig.startPosition.y = j["startPosition"][1].get<float>();
	}
	if (j.contains("startScale") && j["startScale"].is_array() && j["startScale"].size() >= 2) {
		outConfig.startScale.x = j["startScale"][0].get<float>();
		outConfig.startScale.y = j["startScale"][1].get<float>();
	}
	if (j.contains("startRotation") && j["startRotation"].is_number()) outConfig.startRotation = j["startRotation"].get<float>();

	return true;
}

inline bool SavePlayerConfig(const PlayerConfigComponent& config) {
	nlohmann::json j;
	j["moveSpeed"] = config.moveSpeed;
	j["acceleration"] = config.acceleration;
	j["deceleration"] = config.deceleration;
	j["maxSpeed"] = config.maxSpeed;
	j["invincibleDurationDefault"] = config.invincibleDurationDefault;

	j["attackSpawnOffset"] = { config.attackSpawnOffset.x, config.attackSpawnOffset.y };
	j["attackSize"] = { config.attackSize.x, config.attackSize.y };
	j["attackVisibleTime"] = config.attackVisibleTime;
	j["attackLayer"] = config.attackLayer;
	j["attackPower"] = config.attackPower;

	j["playerMaxHp"] = config.playerMaxHp;
	j["invincibleDuration"] = config.invincibleDuration;

	j["flashDuration"] = config.flashDuration;
	j["flashBlinkEnabled"] = config.flashBlinkEnabled;
	j["flashBlinkHz"] = config.flashBlinkHz;
	j["flashMinAlpha"] = config.flashMinAlpha;
	j["flashMaxAlpha"] = config.flashMaxAlpha;
	j["flashColorRGB"] = { config.flashColorRGB.x, config.flashColorRGB.y, config.flashColorRGB.z };

	j["useCameraGateForPlayerHit"] = config.useCameraGateForPlayerHit;
	j["cameraGateNear"] = config.cameraGateNear;
	j["cameraGateDepth"] = config.cameraGateDepth;
	j["cameraGateHalfWidth"] = config.cameraGateHalfWidth;
	j["cameraGateHalfHeight"] = config.cameraGateHalfHeight;

	j["startPosition"] = { config.startPosition.x, config.startPosition.y };
	j["startScale"] = { config.startScale.x, config.startScale.y };
	j["startRotation"] = config.startRotation;

	std::ofstream ofs(MakePlayerConfigFilePath());
	if (!ofs) {
		return false;
	}
	ofs << j.dump(2);
	return true;
}
