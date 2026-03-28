#include "stdafx.h"
#include "SpeechBubbleDataIO.h"
#include <fstream>
#include "externals/nlohmann/json.hpp"

SpeechBubbleConfig SpeechBubbleDataIO::Load(const std::string& path) {
	SpeechBubbleConfig config; // デフォルト値で初期化
	std::ifstream ifs(path);
	if (!ifs.is_open()) {
		return config; // ファイルがなければデフォルト返す
	}
	try {
		nlohmann::json j;
		ifs >> j;
		config = j.get<SpeechBubbleConfig>();
	} catch (...) {
		// パースエラーはデフォルト値で続行
	}
	return config;
}

void SpeechBubbleDataIO::Save(const SpeechBubbleConfig& config, const std::string& path) {
	nlohmann::json j = config;
	std::ofstream ofs(path);
	if (ofs.is_open()) {
		ofs << j.dump(2);
	}
}
