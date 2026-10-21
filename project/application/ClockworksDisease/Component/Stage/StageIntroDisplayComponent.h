#pragma once

// TextComponentと同じEntityに付け、出現までの待ち時間と表示フェードを指定する。
struct StageIntroDisplayComponent {
	float startDelay = 0.5f;
	float fadeInDuration = 0.5f;
	float holdDuration = 2.0f;
	float fadeOutDuration = 0.5f;
	float elapsed = 0.0f;
};
