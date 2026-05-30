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

void RabbitdokuSerializer::GameSave(No::Registry& registry, const No::Vector2& respawnPoint, uint32_t deathCount, uint32_t totalDeath) {
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

	saveData[sceneName]["deathCount"] = deathCount;
	
	saveData[sceneName]["totalDeath"] = totalDeath;
	

	// 上書き保存
	std::ofstream ofs(filePath);
	if (ofs.is_open()) {
		// .dump(4) でインデント幅4の整形された見やすいJSONとして書き出します
		ofs << saveData.dump(4) << std::endl;
		ofs.close();
	}
}

SaveData RabbitdokuSerializer::GameLoad(No::Registry& registry) {
	std::string fileName = "saveData" + std::to_string(sCurrentSaveData) + ".json";
	std::string filePath = sDirectory + fileName;

	std::filesystem::create_directories(sDirectory);


	std::ifstream ifs(filePath);
	if (!ifs.is_open()) {
		return SaveData();
	}

	json saveData;
	try {
		ifs >> saveData;
	}
	catch (const json::parse_error&) {
		ifs.close();
		return SaveData();
	}
	ifs.close();

	std::string sceneName = No::GetCurrentSceneName(registry);
	if (!saveData.contains(sceneName)) {
		// データが存在しない場合は0でリターン
		return SaveData();
	}

	auto& sceneData = saveData[sceneName];

	SaveData data;
	if (sceneData.contains("respawnPoint") && sceneData.contains("deathCount") && sceneData.contains("totalDeath")) {
		data.respawnPoint.x = sceneData["respawnPoint"]["x"].get<float>();
		data.respawnPoint.y = sceneData["respawnPoint"]["y"].get<float>();

		// 死亡数の読み込み
		data.death = sceneData["deathCount"].get<int>();
		data.totalDeath = sceneData["totalDeath"].get<int>();
	}
	
	// 死亡数を加算した状態で上書き保存
	std::ofstream ofs(filePath);
	if (ofs.is_open()) {
		// .dump(4) でインデント幅4の整形された見やすいJSONとして書き出します
		ofs << saveData.dump(4) << std::endl;
		ofs.close();
	}

	return data;
}

SaveData RabbitdokuSerializer::DeleteSave(No::Registry& registry) {
	std::string fileName = "saveData" + std::to_string(sCurrentSaveData) + ".json";
	std::string filePath = sDirectory + fileName;

	std::filesystem::create_directories(sDirectory);


	std::ifstream ifs(filePath);
	if (!ifs.is_open()) {
		return SaveData();
	}

	json saveData;
	try {
		ifs >> saveData;
	}
	catch (const json::parse_error&) {
		ifs.close();
		return SaveData();
	}
	ifs.close();

	std::string sceneName = No::GetCurrentSceneName(registry);
	if (!saveData.contains(sceneName)) {
		// データが存在しない場合は0でリターン
		return SaveData();
	}

	auto& sceneData = saveData[sceneName];

	// SceneNameをオブジェクト名として保存
	saveData[sceneName]["respawnPoint"] = {
		{"x", 0},
		{"y", 0}
	};

	saveData[sceneName]["deathCount"] = 0;

	SaveData data;
	if (sceneData.contains("respawnPoint") && sceneData.contains("deathCount") && sceneData.contains("totalDeath")) {
		data.respawnPoint.x = sceneData["respawnPoint"]["x"].get<float>();
		data.respawnPoint.y = sceneData["respawnPoint"]["y"].get<float>();

		// 死亡数の読み込み
		data.death = sceneData["deathCount"].get<int>();
		data.totalDeath = sceneData["totalDeath"].get<int>();
	}

	std::ofstream ofs(filePath);
	if (ofs.is_open()) {
		// .dump(4) でインデント幅4の整形された見やすいJSONとして書き出します
		ofs << saveData.dump(4) << std::endl;
		ofs.close();
	}


	return data;
}
