#pragma once
#include <cstdint>

/// <summary>
/// スプライトの描画レイヤ―の定義
/// 
/// </summary>
enum class SpriteLayer : uint32_t {
	//背景のスプライト
	Background = 0,
	//背景マップチップ(最奥)
	BackgroundBackMapchip = 1,
	//背景前マップチップ (BackgroundBack の手前)
	BackgroundFrontMapchip = 2,
	//実際の地面などのマップチップ
	FieldMapChip = 3,

	Player = 5,
	Enemy = 6,
	Effect = 7,
};

inline uint32_t ToLayer(SpriteLayer layer) {
	return static_cast<uint32_t>(layer);
}

