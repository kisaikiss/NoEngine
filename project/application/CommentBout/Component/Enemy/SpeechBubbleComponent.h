#pragma once
#include "engine/NoEngine.h"

/// <summary>
/// 吹き出しの演出フェーズ。
///   Appearing : バウンスイージングで最大サイズまで拡大（出現）
///   Stopping  : 最大サイズのまま停止
///   Moving    : ベジェ曲線でボスHPバーへ移動 → 到達でダメージ
/// </summary>
enum class SpeechBubblePhase { Appearing, Stopping, Moving };

/// <summary>
/// 敵撃破時に生成される「吹き出し」の移動状態。
/// SpeechBubbleToBossSystem が二次ベジェで移動し、到達時にボスへダメージを送る。
/// sizeCategory: 0=Large(POW), 1=Medium(BOOM), 2=Small(OH)
/// </summary>
struct SpeechBubbleComponent {
	No::Vector2 start   = { 0.0f, 0.0f };
	No::Vector2 control = { 0.0f, 0.0f };
	No::Vector2 end     = { 0.0f, 0.0f };
	float duration   = 0.9f;
	float elapsed    = 0.0f;
	int   attackPower  = 1;
	int   sizeCategory = 1;  // 0=Large, 1=Medium, 2=Small

	// 演出フェーズ
	SpeechBubblePhase phase        = SpeechBubblePhase::Appearing;
	float             appearElapsed = 0.0f;  // Appearing フェーズ経過時間
	float             stopElapsed   = 0.0f;  // Stopping フェーズ経過時間
};
