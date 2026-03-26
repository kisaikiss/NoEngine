#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// 汎用HPバー表示状態。
/// targetEntity の HealthComponent を参照して表示を更新する
/// </summary>
struct HpBarComponent {
	No::Entity targetEntity = No::nullEntity;
	No::Entity backEntity = No::nullEntity;
	No::Entity delayedEntity = No::nullEntity;
	No::Entity fillEntity = No::nullEntity;

	float displayRatio = 0.0f;
	float delayedRatio = 0.0f;
	float shakeTime = 0.0f;
	float delayedLerpSpeed = 1.8f;
	float prevHp = -1.0f;

	No::Vector2 anchor = { 640.0f, 64.0f };
	No::Vector2 size = { 420.0f, 28.0f };
	No::Color backColor = No::Color(0.1f, 0.1f, 0.1f, 0.75f);
	No::Color delayedColor = No::Color(0.95f, 0.2f, 0.2f, 0.9f);
	No::Color fillColor = No::Color(0.2f, 0.9f, 0.25f, 0.95f);
	int layer = 90;
	int orderBase = 0;
	bool followToZeroWhenDead = true;
};
