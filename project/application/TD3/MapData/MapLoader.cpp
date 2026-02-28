#include "MapLoader.h"
#include "externals/nlohmann/json.hpp"
#include <fstream>
#include <stdexcept>

using json = nlohmann::json;

// ============================================================
//  パス生成ヘルパー
// ============================================================

std::string MapLoader::MakeBasePath(int stageNumber) {
	// 1桁 → "01"、2桁 → "10" のようにゼロ埋め
	std::string stageStr = (stageNumber < 10 ? "0" : "") + std::to_string(stageNumber);
	return "resources/game/td_3105/Stages/stage_" + stageStr;
}

std::string MapLoader::MakeManifestPath(int stageNumber) {
	return MakeBasePath(stageNumber) + ".json";
}

// ============================================================
//  LoadStage（公開 API）
// ============================================================

MapData::StageData MapLoader::LoadStage(const std::string& manifestPath) {
	std::ifstream manifestFile(manifestPath);
	if (!manifestFile.is_open()) {
		//次のマップがないときはここでエラーになる！
		throw std::runtime_error("MapLoader: マニフェストを開けません: " + manifestPath);
	}

	json manifest;
	manifestFile >> manifest;

	std::string connectionMapPath = manifest.at("connection_map").get<std::string>();
	std::string entityMapPath = manifest.at("entity_map").get<std::string>();

	MapData::StageData stageData;
	stageData.connectionMap = LoadConnectionMap(connectionMapPath);
	stageData.entityMap = LoadEntityMap(entityMapPath);
	return stageData;
}

// ============================================================
//  SaveStage（公開 API）
// ============================================================

void MapLoader::SaveStage(const MapData::StageData& data, int stageNumber) {
	std::string base = MakeBasePath(stageNumber);
	std::string manifestPath = base + ".json";
	std::string mapPath = base + "_map.json";
	std::string entitiesPath = base + "_entities.json";

	std::string stageStr = (stageNumber < 10 ? "0" : "") + std::to_string(stageNumber);

	// ---- マニフェスト ----
	{
		json j;
		j["version"] = 1;
		j["stage_name"] = "stage_" + stageStr;
		j["connection_map"] = mapPath;
		j["entity_map"] = entitiesPath;

		std::ofstream f(manifestPath);
		if (!f.is_open()) {
			throw std::runtime_error("MapLoader: マニフェストを書き込めません: " + manifestPath);
		}
		f << j.dump(2);
	}

	// ---- 接続マップ ----
	{
		json j;
		j["stage_name"] = "stage_" + stageStr;
		j["grid_scale"] = data.connectionMap.gridScale;

		json nodes = json::array();
		for (const auto& node : data.connectionMap.nodes) {
			json n;
			n["x"] = node.x;
			n["y"] = node.y;
			n["up"] = node.up;
			n["right"] = node.right;
			n["down"] = node.down;
			n["left"] = node.left;
			n["is_enemy_only"] = node.isEnemyOnly;
			nodes.push_back(n);
		}
		j["nodes"] = nodes;

		std::ofstream f(mapPath);
		if (!f.is_open()) {
			throw std::runtime_error("MapLoader: マップを書き込めません: " + mapPath);
		}
		f << j.dump(2);
	}

	// ---- エンティティ ----
	{
		json j;
		json entities = json::array();
		for (const auto& entity : data.entityMap.entities) {
			json e;
			e["type"] = entity.type;
			e["x"] = entity.x;
			e["y"] = entity.y;
			if (!entity.enemyType.empty()) {
				e["enemy_type"] = entity.enemyType;
			}
			entities.push_back(e);
		}
		j["entities"] = entities;

		std::ofstream f(entitiesPath);
		if (!f.is_open()) {
			throw std::runtime_error("MapLoader: エンティティを書き込めません: " + entitiesPath);
		}
		f << j.dump(2);
	}
}

// ============================================================
//  LoadConnectionMap（内部）
// ============================================================

MapData::ConnectionMapData MapLoader::LoadConnectionMap(const std::string& path) {
	std::ifstream file(path);
	if (!file.is_open()) {
		throw std::runtime_error("MapLoader: 接続マップを開けません: " + path);
	}

	json j;
	file >> j;

	MapData::ConnectionMapData data;
	data.stageName = j.value("stage_name", "");
	data.gridScale = j.value("grid_scale", 1.0f);

	for (const auto& nodeJson : j.at("nodes")) {
		MapData::NodeData node;
		node.x = nodeJson.at("x").get<int>();
		node.y = nodeJson.at("y").get<int>();
		node.up = nodeJson.value("up", false);
		node.right = nodeJson.value("right", false);
		node.down = nodeJson.value("down", false);
		node.left = nodeJson.value("left", false);
		node.isEnemyOnly = nodeJson.value("is_enemy_only", false);
		data.nodes.push_back(node);
	}
	return data;
}

// ============================================================
//  LoadEntityMap（内部）
// ============================================================

MapData::EntityMapData MapLoader::LoadEntityMap(const std::string& path) {
	std::ifstream file(path);
	if (!file.is_open()) {
		throw std::runtime_error("MapLoader: エンティティマップを開けません: " + path);
	}

	json j;
	file >> j;

	MapData::EntityMapData data;
	for (const auto& entityJson : j.at("entities")) {
		MapData::EntityData entity;
		entity.type = entityJson.at("type").get<std::string>();
		entity.x = entityJson.at("x").get<int>();
		entity.y = entityJson.at("y").get<int>();
		entity.enemyType = entityJson.value("enemy_type", "");
		data.entities.push_back(entity);
	}
	return data;
}