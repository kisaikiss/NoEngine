#pragma once
#include <cstdint>

namespace CommentBout {
enum class SpriteLayer : uint32_t {
	Gameplay = 20,
	Effect = 30,
	PauseDim = 900,
	PauseMenuBackground = 905,
	PausePanelLine = 958,
	PauseTitle = 910,
	PauseItem = 920,
	PauseCursor = 930,
};

inline uint32_t ToLayer(SpriteLayer layer) {
	return static_cast<uint32_t>(layer);
}
}
