#pragma once
#include "externals/nlohmann/json.hpp"

/// <summary>
/// プレイヤー死亡カットシーンのパラメータ設定。
/// PlayerDeathSystem が参照し、落下アニメーションの速度と画面外判定を制御する。
/// </summary>
struct PlayerDeathConfig {
	float fallSpeed       = 200.f;  // スクリーン座標/秒 (Y+方向が下)
	float offscreenOffset = 100.f;  // 画面高さ + offset を超えたら画面外と判定
};

inline void to_json(nlohmann::json& j, const PlayerDeathConfig& c) {
	j["fallSpeed"]       = c.fallSpeed;
	j["offscreenOffset"] = c.offscreenOffset;
}
inline void from_json(const nlohmann::json& j, PlayerDeathConfig& c) {
	if (j.contains("fallSpeed"))       c.fallSpeed       = j["fallSpeed"].get<float>();
	if (j.contains("offscreenOffset")) c.offscreenOffset = j["offscreenOffset"].get<float>();
}
