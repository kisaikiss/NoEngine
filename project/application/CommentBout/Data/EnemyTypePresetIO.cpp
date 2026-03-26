#include "stdafx.h"
#include "EnemyTypePresetIO.h"
#include "externals/nlohmann/json.hpp"
#include <fstream>

namespace {
RailEnemyType ParseEnemyType(const std::string& typeName) {
	if (typeName == "MoveAndShoot") {
		return RailEnemyType::MoveAndShoot;
	}
	if (typeName == "Boss") {
		return RailEnemyType::Boss;
	}
	return RailEnemyType::MoveOnly;
}

const char* ToEnemyTypeName(RailEnemyType type) {
	switch (type) {
	case RailEnemyType::MoveAndShoot: return "MoveAndShoot";
	case RailEnemyType::Boss: return "Boss";
	case RailEnemyType::MoveOnly:
	default:
		return "MoveOnly";
	}
}

void EnsurePresetDefaults(EnemyTypePresetMap& presets) {
	if (presets.find(RailEnemyType::MoveOnly) == presets.end()) {
		presets[RailEnemyType::MoveOnly] = EnemyTypePreset{};
	}
	if (presets.find(RailEnemyType::MoveAndShoot) == presets.end()) {
		presets[RailEnemyType::MoveAndShoot] = EnemyTypePreset{};
	}
	if (presets.find(RailEnemyType::Boss) == presets.end()) {
		presets[RailEnemyType::Boss] = EnemyTypePreset{};
	}
}
}

std::string MakeEnemyTypePresetFilePath() {
	return "resources/game/td_3105/RailData/EnemyTypePreset.json";
}

bool LoadEnemyTypePresetMap(EnemyTypePresetMap& outPresets) {
	outPresets.clear();

	std::ifstream ifs(MakeEnemyTypePresetFilePath());
	if (!ifs) {
		EnsurePresetDefaults(outPresets);
		return false;
	}

	nlohmann::json json;
	ifs >> json;
	if (!json.is_object()) {
		EnsurePresetDefaults(outPresets);
		return false;
	}

	for (auto it = json.begin(); it != json.end(); ++it) {
		if (!it.value().is_object()) {
			continue;
		}

		EnemyTypePreset preset;
		const auto& obj = it.value();
		if (obj.contains("modelScale") && obj["modelScale"].is_number()) preset.modelScale = obj["modelScale"].get<float>();
		if (obj.contains("baseColliderBox") && obj["baseColliderBox"].is_array() && obj["baseColliderBox"].size() >= 3) {
			preset.baseColliderBox.x = obj["baseColliderBox"][0].get<float>();
			preset.baseColliderBox.y = obj["baseColliderBox"][1].get<float>();
			preset.baseColliderBox.z = obj["baseColliderBox"][2].get<float>();
		}
		if (obj.contains("shootInterval") && obj["shootInterval"].is_number()) preset.shootInterval = obj["shootInterval"].get<float>();
		if (obj.contains("bulletSpeed") && obj["bulletSpeed"].is_number()) preset.bulletSpeed = obj["bulletSpeed"].get<float>();
		if (obj.contains("bulletDamage") && obj["bulletDamage"].is_number_integer()) preset.bulletDamage = obj["bulletDamage"].get<int>();
		if (obj.contains("bulletLifetime") && obj["bulletLifetime"].is_number()) preset.bulletLifetime = obj["bulletLifetime"].get<float>();
		if (obj.contains("targetDepthFromCamera") && obj["targetDepthFromCamera"].is_number()) preset.targetDepthFromCamera = obj["targetDepthFromCamera"].get<float>();
		if (obj.contains("minHp") && obj["minHp"].is_number_integer()) preset.minHp = obj["minHp"].get<int>();

		outPresets[ParseEnemyType(it.key())] = preset;
	}

	EnsurePresetDefaults(outPresets);
	return true;
}

bool SaveEnemyTypePresetMap(const EnemyTypePresetMap& presets) {
	nlohmann::json json;
	for (const RailEnemyType type : { RailEnemyType::MoveOnly, RailEnemyType::MoveAndShoot, RailEnemyType::Boss }) {
		const EnemyTypePreset preset = GetEnemyTypePresetOrDefault(presets, type);
		auto& node = json[ToEnemyTypeName(type)];
		node["modelScale"] = preset.modelScale;
		node["baseColliderBox"] = { preset.baseColliderBox.x, preset.baseColliderBox.y, preset.baseColliderBox.z };
		node["shootInterval"] = preset.shootInterval;
		node["bulletSpeed"] = preset.bulletSpeed;
		node["bulletDamage"] = preset.bulletDamage;
		node["bulletLifetime"] = preset.bulletLifetime;
		node["targetDepthFromCamera"] = preset.targetDepthFromCamera;
		node["minHp"] = preset.minHp;
	}

	std::ofstream ofs(MakeEnemyTypePresetFilePath());
	if (!ofs) {
		return false;
	}
	ofs << json.dump(2);
	return true;
}

EnemyTypePreset GetEnemyTypePresetOrDefault(const EnemyTypePresetMap& presets, RailEnemyType type) {
	auto it = presets.find(type);
	if (it != presets.end()) {
		return it->second;
	}
	return EnemyTypePreset{};
}
