#pragma once
namespace NoEngine {
namespace Component {
struct Animator2DComponent {
	float animeFrameWidth = 1.f;	// 一コマ分の画像の幅
	float animeFrameHeight = 1.f;	// 一コマ分の画像の高さ
	float frameByFrameTime = 1.f;	// 一コマごとの時間
	uint32_t currentAnimation = 0;  // 何行目のアニメーションか
	uint32_t framesNum = 1;			// 現在の行のフレーム数
	float timer = 0.f;				// 再生時間
	bool isAnimationEnd = false;	// アニメーションが終了したか
	bool stopAnimation = false;		// アニメーションを止めるか
};
}
}