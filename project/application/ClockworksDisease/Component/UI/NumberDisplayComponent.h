#pragma once
#include "engine/NoEngine.h"

// 1桁分のスプライトに付ける目印用コンポーネント（デバッグ識別用、Systemは使わなくても動く）
struct NumberDigitTag {
	uint32_t digitIndex = 0; // 0 = 最上位桁（左端）
};

// 複数桁の数値表示をまとめるコンポーネント。
// digitEntitiesにdigitIndex昇順（左から右）で並んだスプライトEntityを保持しておき、
// NumberDisplaySystemが毎フレームvalueを見て対応するスプライトのUVを書き換える。
struct NumberDisplayComponent {
	uint32_t value = 0;
	uint32_t digitCount = 1;
	std::vector<No::Entity> digitEntities;
};