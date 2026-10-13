#include "stdafx.h"
#include "SystemSceneConfig.h"
#include "SystemCatalog.h"

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

std::optional<std::vector<std::string>> Load(const std::string& sceneName) {
	const std::string path = GetScenePath(sceneName);
	if (path.empty()) return std::nullopt;

	std::ifstream file(path);
	if (!file) return std::nullopt;

	try {
		nlohmann::json data;
		file >> data;
		if (!data.contains("systems") || !data["systems"].is_array()) return std::nullopt;

		std::vector<std::string> names;
		for (const auto& value : data["systems"]) {
			if (!value.is_string()) return std::nullopt;
			std::string name = value.get<std::string>();
			// 1つでも未登録があれば構成全体を採用しない(従来動作を維持)
			if (!SystemCatalog::Get().Contains(name)) return std::nullopt;
			names.push_back(std::move(name));
		}
		return names;
	}
	catch (const std::exception&) {
		LogWarning("Failed to load scene system configuration: " + sceneName);
		return std::nullopt;
	}
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