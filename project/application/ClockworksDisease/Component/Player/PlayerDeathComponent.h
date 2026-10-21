#pragma once
#include "engine/NoEngine.h"

// 落下死パーティクル用。EffectEmitterComponent + TransformComponent を持つEntityに付けておく
struct PlayerDeathEffectTag {};

// 落下死演出の進行状態。死亡した瞬間にPlayerDeathSystemがプレイヤーへ付け、演出終了時に外す。
// 付いている間は移動/ジャンプ系Systemが操作を受け付けない(ロックを兼ねる)。
struct PlayerDeathSequenceComponent {
	enum class Phase {
		kVanish,   // 非表示にしてパーティクルを出し、その場で少し待つ
		kFadeOut,  // 暗転中
		kBlackOut, // 完全に暗転(この直前にリスポーン済み)
		kFadeIn,   // 暗転から復帰
	};

	Phase phase = Phase::kVanish;
	float timer = 0.0f;
	No::Vector3 deathPosition = No::Vector3::ZERO;
	No::Entity overlayEntity = No::INVALID_ENTITY;

	float vanishDuration = 0.4f;
	float fadeOutDuration = 0.5f;
	float blackOutDuration = 0.3f;
	float fadeInDuration = 0.5f;
};
