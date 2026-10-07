#pragma once
#include "engine/NoEngine.h"

struct OpeningPanelComponent {
	// 出現までの時間
	float timeUntilAppearance = 1.0f;
	float appearanceTimer = 0.0f;

	// 出現演出開始から終了までの時間
	float appearanceEffectFinishTime = 1.0f;
	float appearanceEffectTimer = 0.0f;

	// コマの大きさ。Transform2Dで設定したものを演出用に保存する
	No::Vector2 panelScale;
};