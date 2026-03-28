#include "stdafx.h"
#include "EnemyDataIO.h"
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

void EnsureConfigDefaults(EnemyConfigMap& configs) {
    if (configs.find(RailEnemyType::MoveOnly) == configs.end()) {
        configs[RailEnemyType::MoveOnly] = EnemyConfig{};
    }
    if (configs.find(RailEnemyType::MoveAndShoot) == configs.end()) {
        configs[RailEnemyType::MoveAndShoot] = EnemyConfig{};
    }
    if (configs.find(RailEnemyType::Boss) == configs.end()) {
        configs[RailEnemyType::Boss] = EnemyConfig{};
    }
}
}

EnemyConfigMap EnemyDataIO::Load(const std::string& path) {
    EnemyConfigMap configs;

    std::ifstream ifs(path);
    if (!ifs) {
        EnsureConfigDefaults(configs);
        return configs;
    }

    nlohmann::json json;
    ifs >> json;
    if (!json.is_object()) {
        EnsureConfigDefaults(configs);
        return configs;
    }

    for (auto it = json.begin(); it != json.end(); ++it) {
        if (!it.value().is_object()) {
            continue;
        }

        EnemyConfig config;
        const auto& obj = it.value();
        if (obj.contains("modelScale") && obj["modelScale"].is_number()) config.modelScale = obj["modelScale"].get<float>();

        // 旧フォーマット互換: baseColliderBox を enemyCollider.boxSizeMultiplier として読み込む
        if (obj.contains("baseColliderBox") && obj["baseColliderBox"].is_array() && obj["baseColliderBox"].size() >= 3) {
            config.enemyCollider.boxSizeMultiplier.x = obj["baseColliderBox"][0].get<float>();
            config.enemyCollider.boxSizeMultiplier.y = obj["baseColliderBox"][1].get<float>();
            config.enemyCollider.boxSizeMultiplier.z = obj["baseColliderBox"][2].get<float>();
        }
        // 新フォーマット: enemyCollider / bulletCollider
        if (obj.contains("enemyCollider") && obj["enemyCollider"].is_object()) {
            from_json(obj["enemyCollider"], config.enemyCollider);
        }
        if (obj.contains("bulletCollider") && obj["bulletCollider"].is_object()) {
            from_json(obj["bulletCollider"], config.bulletCollider);
        }

        if (obj.contains("shootInterval") && obj["shootInterval"].is_number()) config.shootInterval = obj["shootInterval"].get<float>();
        if (obj.contains("bulletSpeed") && obj["bulletSpeed"].is_number()) config.bulletSpeed = obj["bulletSpeed"].get<float>();
        if (obj.contains("bulletDamage") && obj["bulletDamage"].is_number_integer()) config.bulletDamage = obj["bulletDamage"].get<int>();
        if (obj.contains("bulletLifetime") && obj["bulletLifetime"].is_number()) config.bulletLifetime = obj["bulletLifetime"].get<float>();
        if (obj.contains("targetDepthFromCamera") && obj["targetDepthFromCamera"].is_number()) config.targetDepthFromCamera = obj["targetDepthFromCamera"].get<float>();
        if (obj.contains("shootDistanceMax") && obj["shootDistanceMax"].is_number()) config.shootDistanceMax = obj["shootDistanceMax"].get<float>();
        if (obj.contains("despawnBehindDistance") && obj["despawnBehindDistance"].is_number()) config.despawnBehindDistance = obj["despawnBehindDistance"].get<float>();
        if (obj.contains("minHp") && obj["minHp"].is_number_integer()) config.minHp = obj["minHp"].get<int>();

        configs[ParseEnemyType(it.key())] = config;
    }

    EnsureConfigDefaults(configs);
    return configs;
}

void EnemyDataIO::Save(const EnemyConfigMap& configs, const std::string& path) {
    nlohmann::json json;
    for (const RailEnemyType type : { RailEnemyType::MoveOnly, RailEnemyType::MoveAndShoot, RailEnemyType::Boss }) {
        const EnemyConfig config = GetOrDefault(configs, type);
        auto& node = json[ToEnemyTypeName(type)];
        node["modelScale"] = config.modelScale;
        // 新フォーマットで保存（旧 baseColliderBox キーは書かない）
        to_json(node["enemyCollider"], config.enemyCollider);
        to_json(node["bulletCollider"], config.bulletCollider);
        node["shootInterval"] = config.shootInterval;
        node["bulletSpeed"] = config.bulletSpeed;
        node["bulletDamage"] = config.bulletDamage;
        node["bulletLifetime"] = config.bulletLifetime;
        node["targetDepthFromCamera"] = config.targetDepthFromCamera;
        node["shootDistanceMax"] = config.shootDistanceMax;
        node["despawnBehindDistance"] = config.despawnBehindDistance;
        node["minHp"] = config.minHp;
    }

    std::ofstream ofs(path);
    if (!ofs) {
        return;
    }
    ofs << json.dump(2);
}

EnemyConfig EnemyDataIO::GetOrDefault(const EnemyConfigMap& configs, RailEnemyType type) {
    auto it = configs.find(type);
    if (it != configs.end()) {
        return it->second;
    }
    return EnemyConfig{};
}
