#include "MapLoader.h"
#include "externals/nlohmann/json.hpp"
#include <fstream>
#include <stdexcept>
#include <string>

using json = nlohmann::json;

// ============================================================
//  LoadStage
// ============================================================

MapData::StageData MapLoader::LoadStage(const std::string& manifestPath) {
	// ---- マニフェスト読み込み ----
	std::ifstream manifestFile(manifestPath);
	if (!manifestFile.is_open()) {
		throw std::runtime_error("MapLoader: マニフェストを開けません: " + manifestPath);
	}

	json manifest;
	manifestFile >> manifest;

	// マニフェストから各マップファイルのパスを取得
	std::string connectionMapPath = manifest.at("connection_map").get<std::string>();
	std::string entityMapPath = manifest.at("entity_map").get<std::string>();

	// ---- 各マップを読み込んで StageData にまとめる ----
	MapData::StageData stageData;
	stageData.connectionMap = LoadConnectionMap(connectionMapPath);
	stageData.entityMap = LoadEntityMap(entityMapPath);

	return stageData;
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

	// ---- メタ情報 ----
	data.stageName = j.value("stage_name", "");
	data.gridScale = j.value("grid_scale", 1.0f);

	// ---- ノード配列 ----
	for (const auto& nodeJson : j.at("nodes")) {
		MapData::NodeData node;
		node.x = nodeJson.at("x").get<int>();
		node.y = nodeJson.at("y").get<int>();
		node.up = nodeJson.value("up", false);
		node.right = nodeJson.value("right", false);
		node.down = nodeJson.value("down", false);
		node.left = nodeJson.value("left", false);
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

	// ---- エンティティ配列 ----
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
