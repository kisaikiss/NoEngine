#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// 被ダメ時フラッシュの共通状態
/// </summary>
struct DamageFlashComponent {
	float timer = 0.0f;
	float duration = 0.16f;
	No::Color flashColor = No::Color(1.0f, 0.35f, 0.35f, 0.9f);
	No::Color baseColor = No::Color(1.0f, 1.0f, 1.0f, 1.0f);
	bool affectSprite = true;
	bool affectMaterial = true;
};
