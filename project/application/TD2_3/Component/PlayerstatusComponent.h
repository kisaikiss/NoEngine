#pragma once
#include <array>

struct PlayerStatusComponent
{
	static inline int32_t score = 0;
	float scoreRatio = 1.0f;
	bool isComboing = false;
	int32_t level = 1;
	int32_t exp = 0;
	// 現在のHP（ゲーム内で変動）と上限を分離
	int32_t hp = 5;
	int32_t hpMax = 5;
	int32_t requiredExp = 5;
	int32_t ballCount = 1;
	// レベルアップ選択中か（UI が開かれているか）を示すフラグ
	bool pendingUpgrade = false;
};