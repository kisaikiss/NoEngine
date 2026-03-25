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
}

std::string MakeEnemyTypePresetFilePath() {
	return "resources/game/td_3105/RailData/EnemyTypePreset.json";
}

bool LoadEnemyTypePresetMap(EnemyTypePresetMap& outPresets) {
	outPresets.clear();

	std::ifstream ifs(MakeEnemyTypePresetFilePath());
	if (!ifs) {
		return false;
	}

	nlohmann::json json;
	ifs >> json;
	if (!json.is_object()) {
		return false;
	}

	for (auto it = json.begin(); it != json.end(); ++it) {
		if (!it.value().is_object()) {
			continue;
		}

		EnemyTypePreset preset;
		const auto& obj = it.value();
		if (obj.contains("modelScale") && obj["modelScale"].is_number()) preset.modelScale = obj["modelScale"].get<float>();
		if (obj.contains("shootInterval") && obj["shootInterval"].is_number()) preset.shootInterval = obj["shootInterval"].get<float>();
		if (obj.contains("bulletSpeed") && obj["bulletSpeed"].is_number()) preset.bulletSpeed = obj["bulletSpeed"].get<float>();
		if (obj.contains("bulletDamage") && obj["bulletDamage"].is_number_integer()) preset.bulletDamage = obj["bulletDamage"].get<int>();
		if (obj.contains("bulletLifetime") && obj["bulletLifetime"].is_number()) preset.bulletLifetime = obj["bulletLifetime"].get<float>();
		if (obj.contains("targetDepthFromCamera") && obj["targetDepthFromCamera"].is_number()) preset.targetDepthFromCamera = obj["targetDepthFromCamera"].get<float>();
		if (obj.contains("minHp") && obj["minHp"].is_number_integer()) preset.minHp = obj["minHp"].get<int>();

		outPresets[ParseEnemyType(it.key())] = preset;
	}

	return true;
}

EnemyTypePreset GetEnemyTypePresetOrDefault(const EnemyTypePresetMap& presets, RailEnemyType type) {
	auto it = presets.find(type);
	if (it != presets.end()) {
		return it->second;
	}
	return EnemyTypePreset{};
}
