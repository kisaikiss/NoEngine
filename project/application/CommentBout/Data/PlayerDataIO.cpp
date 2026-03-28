#include "stdafx.h"
#include "PlayerDataIO.h"
#include "externals/nlohmann/json.hpp"
#include <fstream>

PlayerConfig PlayerDataIO::Load(const std::string& path) {
    PlayerConfig config;

    std::ifstream ifs(path);
    if (!ifs) {
        return config;
    }

    nlohmann::json j;
    ifs >> j;
    if (!j.is_object()) {
        return config;
    }

    if (j.contains("moveSpeed") && j["moveSpeed"].is_number()) config.moveSpeed = j["moveSpeed"].get<float>();
    if (j.contains("acceleration") && j["acceleration"].is_number()) config.acceleration = j["acceleration"].get<float>();
    if (j.contains("deceleration") && j["deceleration"].is_number()) config.deceleration = j["deceleration"].get<float>();
    if (j.contains("maxSpeed") && j["maxSpeed"].is_number()) config.maxSpeed = j["maxSpeed"].get<float>();
    if (j.contains("invincibleDurationDefault") && j["invincibleDurationDefault"].is_number()) config.invincibleDurationDefault = j["invincibleDurationDefault"].get<float>();

    if (j.contains("attackSpawnOffset") && j["attackSpawnOffset"].is_array() && j["attackSpawnOffset"].size() >= 2) {
        config.attackSpawnOffset.x = j["attackSpawnOffset"][0].get<float>();
        config.attackSpawnOffset.y = j["attackSpawnOffset"][1].get<float>();
    }
    if (j.contains("attackSize") && j["attackSize"].is_array() && j["attackSize"].size() >= 2) {
        config.attackSize.x = j["attackSize"][0].get<float>();
        config.attackSize.y = j["attackSize"][1].get<float>();
    }
    if (j.contains("attackVisibleTime") && j["attackVisibleTime"].is_number()) config.attackVisibleTime = j["attackVisibleTime"].get<float>();
    if (j.contains("attackLayer") && j["attackLayer"].is_number_integer()) config.attackLayer = j["attackLayer"].get<int>();
    if (j.contains("attackPower") && j["attackPower"].is_number_integer()) config.attackPower = j["attackPower"].get<int>();

    if (j.contains("playerMaxHp") && j["playerMaxHp"].is_number_integer()) config.playerMaxHp = j["playerMaxHp"].get<int>();
    if (j.contains("invincibleDuration") && j["invincibleDuration"].is_number()) config.invincibleDuration = j["invincibleDuration"].get<float>();

    if (j.contains("flashDuration") && j["flashDuration"].is_number()) config.flashDuration = j["flashDuration"].get<float>();
    if (j.contains("flashBlinkEnabled") && j["flashBlinkEnabled"].is_boolean()) config.flashBlinkEnabled = j["flashBlinkEnabled"].get<bool>();
    if (j.contains("flashBlinkHz") && j["flashBlinkHz"].is_number()) config.flashBlinkHz = j["flashBlinkHz"].get<float>();
    if (j.contains("flashMinAlpha") && j["flashMinAlpha"].is_number()) config.flashMinAlpha = j["flashMinAlpha"].get<float>();
    if (j.contains("flashMaxAlpha") && j["flashMaxAlpha"].is_number()) config.flashMaxAlpha = j["flashMaxAlpha"].get<float>();
    if (j.contains("flashColorRGB") && j["flashColorRGB"].is_array() && j["flashColorRGB"].size() >= 3) {
        config.flashColorRGB.x = j["flashColorRGB"][0].get<float>();
        config.flashColorRGB.y = j["flashColorRGB"][1].get<float>();
        config.flashColorRGB.z = j["flashColorRGB"][2].get<float>();
    }

    if (j.contains("useCameraGateForPlayerHit") && j["useCameraGateForPlayerHit"].is_boolean()) config.useCameraGateForPlayerHit = j["useCameraGateForPlayerHit"].get<bool>();
    if (j.contains("cameraGateNear") && j["cameraGateNear"].is_number()) config.cameraGateNear = j["cameraGateNear"].get<float>();
    if (j.contains("cameraGateDepth") && j["cameraGateDepth"].is_number()) config.cameraGateDepth = j["cameraGateDepth"].get<float>();
    if (j.contains("cameraGateHalfWidth") && j["cameraGateHalfWidth"].is_number()) config.cameraGateHalfWidth = j["cameraGateHalfWidth"].get<float>();
    if (j.contains("cameraGateHalfHeight") && j["cameraGateHalfHeight"].is_number()) config.cameraGateHalfHeight = j["cameraGateHalfHeight"].get<float>();

    if (j.contains("startPosition") && j["startPosition"].is_array() && j["startPosition"].size() >= 2) {
        config.startPosition.x = j["startPosition"][0].get<float>();
        config.startPosition.y = j["startPosition"][1].get<float>();
    }
    if (j.contains("startScale") && j["startScale"].is_array() && j["startScale"].size() >= 2) {
        config.startScale.x = j["startScale"][0].get<float>();
        config.startScale.y = j["startScale"][1].get<float>();
    }
    if (j.contains("startRotation") && j["startRotation"].is_number()) config.startRotation = j["startRotation"].get<float>();

    // ColliderConfig フィールドの読み込み
    if (j.contains("playerCollider2D") && j["playerCollider2D"].is_object()) {
        from_json(j["playerCollider2D"], config.playerCollider2D);
    }
    if (j.contains("attackCollider") && j["attackCollider"].is_object()) {
        from_json(j["attackCollider"], config.attackCollider);
    }

    return config;
}

void PlayerDataIO::Save(const PlayerConfig& config, const std::string& path) {
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

    // ColliderConfig フィールドの書き込み
    to_json(j["playerCollider2D"], config.playerCollider2D);
    to_json(j["attackCollider"], config.attackCollider);

    std::ofstream ofs(path);
    if (!ofs) {
        return;
    }
    ofs << j.dump(2);
}
