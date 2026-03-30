#pragma once
#include "engine/NoEngine.h"

struct BossDefeatSequenceComponent {
	enum class Phase {
		Inactive = 0,
		PreExplosionWait,   // 爆発前の待機
		Explosion,          // BOOM スプライト表示中
		PostExplosionWait,  // 爆発後の余韻
		Done                // 完了
	};

	Phase phase = Phase::Inactive;
	float phaseTimer = 0.f;

	float preExplosionDuration  = 0.3f;
	float explosionDuration     = 0.8f;
	float postExplosionDuration = 0.5f;

	No::Entity bossEntity         = No::nullEntity;
	No::Entity effectSpriteEntity = No::nullEntity;

	bool cutsceneStarted = false;  // カットシーン開始フラグ（inCutscene/isPause を一度だけセットするため）
};
