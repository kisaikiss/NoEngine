#pragma once
#include "engine/NoEngine.h"
#include "externals/nlohmann/json.hpp"

/// <summary>
/// 吹き出し（SpeechBubble）の1サイズ分の設定
/// </summary>
struct SpeechBubbleSizeConfig {
	No::Vector2 spriteSize{ 80.f, 80.f };
	int   attackPower = 1;
	float duration    = 0.9f;
};

/// <summary>
/// 吹き出し全体の設定。
/// カメラからの3D距離で大中小を決定する。
/// largeMaxDistance: 0 〜 largeMax  → Large (POW.png)
/// mediumMaxDistance: largeMax 〜 mediumMax → Medium (BOOM.png)
/// それ以上 → Small (OH.png)
/// </summary>
struct SpeechBubbleConfig {
	float largeMaxDistance  = 8.0f;
	float mediumMaxDistance = 16.0f;

	SpeechBubbleSizeConfig sizeLarge  = { { 120.f, 120.f }, 3, 0.9f };
	SpeechBubbleSizeConfig sizeMedium = { {  80.f,  80.f }, 2, 0.9f };
	SpeechBubbleSizeConfig sizeSmall  = { {  48.f,  48.f }, 1, 0.9f };
};

// ---------- JSON serialization ----------

inline void to_json(nlohmann::json& j, const SpeechBubbleSizeConfig& s) {
	j["spriteSize"]  = { s.spriteSize.x, s.spriteSize.y };
	j["attackPower"] = s.attackPower;
	j["duration"]    = s.duration;
}
inline void from_json(const nlohmann::json& j, SpeechBubbleSizeConfig& s) {
	if (j.contains("spriteSize") && j["spriteSize"].is_array() && j["spriteSize"].size() >= 2)
		s.spriteSize = { j["spriteSize"][0], j["spriteSize"][1] };
	if (j.contains("attackPower")) s.attackPower = j["attackPower"].get<int>();
	if (j.contains("duration"))    s.duration    = j["duration"].get<float>();
}

inline void to_json(nlohmann::json& j, const SpeechBubbleConfig& c) {
	j["largeMaxDistance"]  = c.largeMaxDistance;
	j["mediumMaxDistance"] = c.mediumMaxDistance;
	j["sizeLarge"]  = c.sizeLarge;
	j["sizeMedium"] = c.sizeMedium;
	j["sizeSmall"]  = c.sizeSmall;
}
inline void from_json(const nlohmann::json& j, SpeechBubbleConfig& c) {
	if (j.contains("largeMaxDistance"))  c.largeMaxDistance  = j["largeMaxDistance"].get<float>();
	if (j.contains("mediumMaxDistance")) c.mediumMaxDistance = j["mediumMaxDistance"].get<float>();
	if (j.contains("sizeLarge"))  c.sizeLarge  = j["sizeLarge"].get<SpeechBubbleSizeConfig>();
	if (j.contains("sizeMedium")) c.sizeMedium = j["sizeMedium"].get<SpeechBubbleSizeConfig>();
	if (j.contains("sizeSmall"))  c.sizeSmall  = j["sizeSmall"].get<SpeechBubbleSizeConfig>();
}
