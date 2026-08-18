#pragma once
#include "engine/NoEngine.h"

// TransformRoutine演出中、FollowCameraSystemの追従を止めておくためのタグ。
// 演出が終わったタイミングでCameraIntroSystemがこれを外し、以降FollowCameraSystemが追従を再開する。
struct CameraIntroLockTag {};

struct CameraIntroComponent {
	enum class Phase {
		kPlayingRoutine, // TransformRoutineComponentの再生完了を待っている
		kFadeOut,        // 画面を暗転させている
		kBlackOut,		 // 完全に暗転した状態
		kFadeIn,         // 暗転から復帰している
		kDone,           // 演出終了
	};

	Phase phase = Phase::kPlayingRoutine;
	float fadeOutDuration = 0.5f; // 暗転にかかる時間(秒)
	float blackOutDuration = 0.5f; // 暗転中に待つ時間
	float fadeInDuration = 0.5f;  // 復帰にかかる時間(秒)

	// エディタでPlayして確認するたびに演出を見るのが煩わしい場合はfalseにする。
	// falseの場合、エディタビルド実行時のみ演出全体を即座にスキップする
	// (製品ビルドでは常にtrue相当の動作＝必ず演出を再生する)
	bool playInEditor = true;

	// 以下はランタイム用（エディタでは基本触らない想定）
	float fadeTimer = 0.0f;
	No::Entity overlayEntity = No::INVALID_ENTITY;
};