#pragma once
#include "engine/NoEngine.h"
#include "externals/nlohmann/json.hpp"

struct ClearOverConfigComponent {
	// フェード設定
	float fadeDuration    = 0.5f;
	float fadeTargetAlpha = 0.85f;

	// ロゴ設定
	No::Vector2 logoStartPos{ 640.f, 800.f };
	No::Vector2 logoEndPos  { 640.f, 250.f };
	No::Vector2 logoSize    { 400.f, 100.f };
	float logoAppearDuration = 0.4f;
	int   logoEaseType = 2;  // 2 = EaseOutCubic

	// メニュー設定 (ポーズと同様の構造)
	No::Vector2 itemBaseStartPos{ 640.f, 900.f };
	No::Vector2 itemBaseEndPos  { 640.f, 420.f };
	No::Vector2 itemSize        { 340.f, 70.f };
	float itemSpacing        = 80.f;
	float menuAppearDuration = 0.3f;

	// 確認アニメーション
	float confirmDuration = 0.12f;
	float selectedScale   = 1.08f;
	float confirmScale    = 1.18f;
};

// ---------- JSON serialization ----------

inline void to_json(nlohmann::json& j, const ClearOverConfigComponent& c) {
	j["fadeDuration"]        = c.fadeDuration;
	j["fadeTargetAlpha"]     = c.fadeTargetAlpha;
	j["logoStartPos"]        = { c.logoStartPos.x, c.logoStartPos.y };
	j["logoEndPos"]          = { c.logoEndPos.x, c.logoEndPos.y };
	j["logoSize"]            = { c.logoSize.x, c.logoSize.y };
	j["logoAppearDuration"]  = c.logoAppearDuration;
	j["logoEaseType"]        = c.logoEaseType;
	j["itemBaseStartPos"]    = { c.itemBaseStartPos.x, c.itemBaseStartPos.y };
	j["itemBaseEndPos"]      = { c.itemBaseEndPos.x, c.itemBaseEndPos.y };
	j["itemSize"]            = { c.itemSize.x, c.itemSize.y };
	j["itemSpacing"]         = c.itemSpacing;
	j["menuAppearDuration"]  = c.menuAppearDuration;
	j["confirmDuration"]     = c.confirmDuration;
	j["selectedScale"]       = c.selectedScale;
	j["confirmScale"]        = c.confirmScale;
}

inline void from_json(const nlohmann::json& j, ClearOverConfigComponent& c) {
	auto getF  = [&](const char* key, float& v)  { if (j.contains(key)) v = j[key].get<float>(); };
	auto getI  = [&](const char* key, int& v)    { if (j.contains(key)) v = j[key].get<int>(); };
	auto getV2 = [&](const char* key, No::Vector2& v) {
		if (j.contains(key) && j[key].is_array() && j[key].size() >= 2) {
			v = { j[key][0].get<float>(), j[key][1].get<float>() };
		}
	};
	getF("fadeDuration",       c.fadeDuration);
	getF("fadeTargetAlpha",    c.fadeTargetAlpha);
	getV2("logoStartPos",      c.logoStartPos);
	getV2("logoEndPos",        c.logoEndPos);
	getV2("logoSize",          c.logoSize);
	getF("logoAppearDuration", c.logoAppearDuration);
	getI("logoEaseType",       c.logoEaseType);
	getV2("itemBaseStartPos",  c.itemBaseStartPos);
	getV2("itemBaseEndPos",    c.itemBaseEndPos);
	getV2("itemSize",          c.itemSize);
	getF("itemSpacing",        c.itemSpacing);
	getF("menuAppearDuration", c.menuAppearDuration);
	getF("confirmDuration",    c.confirmDuration);
	getF("selectedScale",      c.selectedScale);
	getF("confirmScale",       c.confirmScale);
}
