#include "stdafx.h"
#include "SystemSceneConfig.h"

#include "externals/nlohmann/json.hpp"

namespace NoEngine {
namespace ECS {
namespace SystemSceneConfig {

namespace {
const char* kSceneDirectory = "resources/game/Scenes/";

bool IsSafeSceneName(const std::string& name) {
	return !name.empty() && name.find_first_of("/\\:") == std::string::npos;
}
}

std::string GetScenePath(const std::string& sceneName) {
	if (!IsSafeSceneName(sceneName)) return std::string();
	return std::string(kSceneDirectory) + sceneName + ".json";
}

bool Save(const std::string& sceneName, const std::vector<std::string>& systemNames) {
	const std::string path = GetScenePath(sceneName);
	if (path.empty()) return false;

	nlohmann::json data = nlohmann::json::object();
	{
		std::ifstream in(path);
		if (in) {
			try {
				in >> data;
			}
			catch (const std::exception&) {
				return false; // 壊れたJSONを上書きしない
			}
		}
	}
	data["systems"] = systemNames;

	std::ofstream out(path);
	if (!out) return false;
	out << data.dump(4);
	return true;
}

}
}
}