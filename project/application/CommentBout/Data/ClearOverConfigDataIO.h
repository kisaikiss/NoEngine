#pragma once
#include "application/CommentBout/Component/ClearOverConfigComponent.h"
#include <string>

class ClearOverConfigDataIO {
public:
	static constexpr const char* kDefaultPath =
		"resources/game/td_3105/Data/Config/ClearOverConfig.json";

	static ClearOverConfigComponent Load(const std::string& path = kDefaultPath);
	static void Save(const ClearOverConfigComponent& config, const std::string& path = kDefaultPath);
};
