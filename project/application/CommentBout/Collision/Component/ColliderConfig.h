#pragma once
#include "engine/NoEngine.h"
#include "externals/nlohmann/json.hpp"

/// <summary>
/// 汎用コライダー設定。PlayerConfig / EnemyConfig などに内包して使用する。
/// </summary>
struct ColliderConfig {
    No::Vector3 localOffset3D{ 0.f, 0.f, 0.f };      // 3Dコライダーのローカルオフセット
    No::Vector2 localOffset2D{ 0.f, 0.f };            // 2Dコライダーのローカルオフセット
    float       radiusMultiplier   = 1.0f;            // 球半径の倍率 (Sphere用)
    No::Vector3 boxSizeMultiplier  { 1.f, 1.f, 1.f }; // ボックスサイズの倍率 (Box用)
    No::Vector2 sizeMultiplier2D   { 1.f, 1.f };      // 2Dサイズの倍率
};

inline void to_json(nlohmann::json& j, const ColliderConfig& c) {
    j["localOffset3D"]      = { c.localOffset3D.x, c.localOffset3D.y, c.localOffset3D.z };
    j["localOffset2D"]      = { c.localOffset2D.x, c.localOffset2D.y };
    j["radiusMultiplier"]   = c.radiusMultiplier;
    j["boxSizeMultiplier"]  = { c.boxSizeMultiplier.x, c.boxSizeMultiplier.y, c.boxSizeMultiplier.z };
    j["sizeMultiplier2D"]   = { c.sizeMultiplier2D.x, c.sizeMultiplier2D.y };
}

inline void from_json(const nlohmann::json& j, ColliderConfig& c) {
    if (j.contains("localOffset3D") && j["localOffset3D"].is_array() && j["localOffset3D"].size() >= 3)
        c.localOffset3D = { j["localOffset3D"][0], j["localOffset3D"][1], j["localOffset3D"][2] };
    if (j.contains("localOffset2D") && j["localOffset2D"].is_array() && j["localOffset2D"].size() >= 2)
        c.localOffset2D = { j["localOffset2D"][0], j["localOffset2D"][1] };
    if (j.contains("radiusMultiplier"))  c.radiusMultiplier  = j["radiusMultiplier"];
    if (j.contains("boxSizeMultiplier") && j["boxSizeMultiplier"].is_array() && j["boxSizeMultiplier"].size() >= 3)
        c.boxSizeMultiplier = { j["boxSizeMultiplier"][0], j["boxSizeMultiplier"][1], j["boxSizeMultiplier"][2] };
    if (j.contains("sizeMultiplier2D") && j["sizeMultiplier2D"].is_array() && j["sizeMultiplier2D"].size() >= 2)
        c.sizeMultiplier2D = { j["sizeMultiplier2D"][0], j["sizeMultiplier2D"][1] };
}
