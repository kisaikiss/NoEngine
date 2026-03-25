#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// ボスHPバー表示のランタイム状態。
/// BossHpBarViewSystem が通常ゲージ/遅延赤ゲージ/シェイクを更新する
/// </summary>
struct BossHpBarComponent {
	No::Entity bossEntity = No::nullEntity;
	No::Entity backEntity = No::nullEntity;
	No::Entity delayedEntity = No::nullEntity;
	No::Entity fillEntity = No::nullEntity;

	float displayRatio = 1.0f;
	float delayedRatio = 1.0f;
	float shakeTime = 0.0f;
	float delayedLerpSpeed = 1.8f;
	float prevHp = -1.0f;
	No::Vector2 anchor = { 640.0f, 64.0f };
	No::Vector2 size = { 420.0f, 28.0f };
};
