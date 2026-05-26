#include "stdafx.h"
#include "RabbitdokuSerializer.h"

using json = nlohmann::json;

namespace {
uint32_t sCurrentSaveData = 1;
std::string sDirectory = "resources/game/RabbitdokuOdyssey3Plus/SaveData/";
}

void RabbitdokuSerializer::SetSaveData(uint32_t saveDataNum) {
	sCurrentSaveData = saveDataNum;
}

void RabbitdokuSerializer::GameSave(No::Registry& registry, const No::Vector2& respawnPoint) {
	std::string fileName = "saveData" + std::to_string(sCurrentSaveData) + ".json";
	std::string filePath = sDirectory + fileName;

	std::filesystem::create_directories(sDirectory);

	json saveData;
	// 既存のJSONファイルがあれば読み込む
	std::ifstream ifs(filePath);
	if (ifs.is_open()) {
		try {
			ifs >> saveData;
		}
		catch (const json::parse_error&) {
			// ファイルが空だったり、壊れている場合は初期化
			saveData = json::object();
		}
		ifs.close();
	} else {
		// ファイルが存在しない場合は空のオブジェクトとして初期化
		saveData = json::object();
	}

	std::string sceneName = No::GetCurrentSceneName(registry);

	// SceneNameをオブジェクト名として保存
	saveData[sceneName]["respawnPoint"] = {
		{"x", respawnPoint.x},
		{"y", respawnPoint.y}
	};

	if (!saveData[sceneName].contains("deathCount")) {
		saveData[sceneName]["deathCount"] = 0;
	}

	// 上書き保存
	std::ofstream ofs(filePath);
	if (ofs.is_open()) {
		// .dump(4) でインデント幅4の整形された見やすいJSONとして書き出します
		ofs << saveData.dump(4) << std::endl;
		ofs.close();
	}
}

No::Vector2 RabbitdokuSerializer::GameLoad(No::Registry& registry) {
	std::string fileName = "saveData" + std::to_string(sCurrentSaveData) + ".json";
	std::string filePath = sDirectory + fileName;

	std::filesystem::create_directories(sDirectory);


	std::ifstream ifs(filePath);
	if (!ifs.is_open()) {
		return No::Vector2::ZERO;
	}

	json saveData;
	try {
		ifs >> saveData;
	}
	catch (const json::parse_error&) {
		ifs.close();
		return No::Vector2::ZERO;
	}
	ifs.close();

	std::string sceneName = No::GetCurrentSceneName(registry);
	if (!saveData.contains(sceneName)) {
		// データが存在しない場合は0でリターン
		return No::Vector2::ZERO;
	}

	auto& sceneData = saveData[sceneName];

	No::Vector2 respawnPoint{};
	if (sceneData.contains("respawnPoint") && sceneData.contains("deathCount")) {
		respawnPoint.x = sceneData["respawnPoint"]["x"].get<float>();
		respawnPoint.y = sceneData["respawnPoint"]["y"].get<float>();

		// 死亡数の読み込み
		int count = sceneData["deathCount"].get<int>();
		count++;
		sceneData["deathCount"] = count;
	}
	
	// 死亡数を加算した状態で上書き保存
	std::ofstream ofs(filePath);
	if (ofs.is_open()) {
		// .dump(4) でインデント幅4の整形された見やすいJSONとして書き出します
		ofs << saveData.dump(4) << std::endl;
		ofs.close();
	}

	return respawnPoint;
}
