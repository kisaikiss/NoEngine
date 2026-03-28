#pragma once
#include "application/CommentBout/Data/SpeechBubbleConfig.h"
#include <string>

class SpeechBubbleDataIO {
public:
	static constexpr const char* kDefaultPath =
		"resources/game/td_3105/Data/Config/SpeechBubbleConfig.json";

	static SpeechBubbleConfig Load(const std::string& path = kDefaultPath);
	static void Save(const SpeechBubbleConfig& config, const std::string& path = kDefaultPath);
};
