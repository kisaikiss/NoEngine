#pragma once
#include "application/CommentBout/Data/SpeechBubbleConfig.h"
#include <string>

class SpeechBubbleDataIO {
public:
	// Phase 3 でパス移動後はここを変更するだけでよい
	// TODO(Phase3): "resources/game/td_3105/Data/Config/SpeechBubbleConfig.json" に移動
	static constexpr const char* kDefaultPath =
		"resources/game/td_3105/RailData/SpeechBubbleConfig.json";

	static SpeechBubbleConfig Load(const std::string& path = kDefaultPath);
	static void Save(const SpeechBubbleConfig& config, const std::string& path = kDefaultPath);
};
