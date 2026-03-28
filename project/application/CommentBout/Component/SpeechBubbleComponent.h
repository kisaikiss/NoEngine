#pragma once
#include "engine/NoEngine.h"

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
};
