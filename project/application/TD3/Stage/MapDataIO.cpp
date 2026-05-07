#include "application/TD3/Stage/MapDataIO.h"
#include <externals/nlohmann/json.hpp>
#include <fstream>
#include <filesystem>

using json = nlohmann::json;

namespace Stage {
	namespace {
		json SerializeCell(const TileCell& c) {
			if (c.IsEmpty()) {
				return nullptr;
			}
			return json::array({ c.textureIdx, c.tileX, c.tileY });
		}

		TileCell DeserializeCell(const json& j) {
			if (j.is_null()) {
				return TileCell{};
			}
			if (!j.is_array() || j.size() != 3) {
				return TileCell{};
			}
			return TileCell{ j[0].get<int>(), j[1].get<int>(), j[2].get<int>() };
		}
	}

	std::string MapDataIO::BuildFilePath(const std::string& mapName,
		const std::string& basePath) {
		// mapName = "Stage01_A"
		// stageDir = "Stage01" (アンダーバーより前)
		std::string stageDir = mapName.substr(0, mapName.find('_'));
		return basePath + "/" + stageDir + "/" + mapName + ".json";
	}

	bool MapDataIO::Save(const MapData& data, const std::string& basePath) {
		std::string filePath = BuildFilePath(data.GetMapName(), basePath);

		// 保存先ディレクトリを作成（存在しない場合）
		std::filesystem::path fsPath(filePath);
		std::error_code ec;
		std::filesystem::create_directories(fsPath.parent_path(), ec);
		if (ec) return false;

		json j;
		j["stageNo"] = data.stageNo;
		j["mapId"] = static_cast<int>(data.mapId);
		j["width"] = data.width;
		j["height"] = data.height;

		j["fieldTiles"] = json::array();
		for (const auto& row : data.fieldTiles) {
			json rowArr = json::array();
			for (const auto& cell : row) {
				rowArr.push_back(SerializeCell(cell));
			}
			j["fieldTiles"].push_back(rowArr);
		}

		j["bgTilesFront"] = json::array();
		for (const auto& row : data.bgTilesFront) {
			json rowArr = json::array();
			for (const auto& cell : row) { rowArr.push_back(SerializeCell(cell)); }
			j["bgTilesFront"].push_back(rowArr);
		}

		j["bgTilesBack"] = json::array();
		for (const auto& row : data.bgTilesBack) {
			json rowArr = json::array();
			for (const auto& cell : row) { rowArr.push_back(SerializeCell(cell)); }
			j["bgTilesBack"].push_back(rowArr);
		}

		json objects = json::array();
		for (const auto& obj : data.objects) {
			objects.push_back({
				{"type", obj.type},
				{"x",    obj.x},
				{"y",    obj.y}
				});
		}
		j["objects"] = objects;

		j["initialSpawn"] = { {"x", data.initialSpawn.x}, {"y", data.initialSpawn.y} };

		json respawnPoints = json::array();
		for (const auto& rp : data.respawnPoints) {
			respawnPoints.push_back({
				{"id", rp.id},
				{"x",  rp.x},
				{"y",  rp.y}
				});
		}
		j["respawnPoints"] = respawnPoints;

		json transitions = json::array();
		for (const auto& tp : data.transitions) {
			transitions.push_back({
				{"id",			tp.id},
				{"x",			tp.x},
				{"y",			tp.y},
				{"targetMap",	tp.targetMap},
				{"targetPoint",	tp.targetPoint}
				});
		}
		j["transitions"] = transitions;

		std::ofstream ofs(filePath);
		if (!ofs) return false;
		ofs << j.dump(2);
		return true;
	}

	bool MapDataIO::Load(MapData& data, const std::string& mapName,
		const std::string& basePath) {
		std::string filePath = BuildFilePath(mapName, basePath);

		std::ifstream ifs(filePath);
		if (!ifs) return false;

		json j = json::parse(ifs, nullptr, /*exceptions=*/false);
		if (j.is_discarded()) return false;

		data.stageNo = j["stageNo"].get<int>();
		data.mapId = static_cast<char>(j["mapId"].get<int>());
		data.width = j["width"].get<int>();
		data.height = j["height"].get<int>();

		data.fieldTiles.clear();
		for (const auto& jrow : j["fieldTiles"]) {
			std::vector<TileCell> row;
			row.reserve(jrow.size());
			for (const auto& jcell : jrow) {
				row.push_back(DeserializeCell(jcell));
			}
			data.fieldTiles.push_back(std::move(row));
		}

		auto loadLayer = [&](std::vector<std::vector<TileCell>>& tiles, const char* key) {
			tiles.clear();
			if (!j.contains(key)) return;
			for (const auto& jrow : j[key]) {
				std::vector<TileCell> row;
				row.reserve(jrow.size());
				for (const auto& jcell : jrow) { row.push_back(DeserializeCell(jcell)); }
				tiles.push_back(std::move(row));
			}
			};
		loadLayer(data.bgTilesFront, "bgTilesFront");
		loadLayer(data.bgTilesBack, "bgTilesBack");

		if (j.contains("initialSpawn")) {
			data.initialSpawn.x = j["initialSpawn"]["x"].get<int>();
			data.initialSpawn.y = j["initialSpawn"]["y"].get<int>();
		}

		data.respawnPoints.clear();
		if (j.contains("respawnPoints")) {
			for (const auto& rp : j["respawnPoints"]) {
				Stage::RespawnPoint r;
				r.id = rp["id"].get<std::string>();
				r.x = rp["x"].get<int>();
				r.y = rp["y"].get<int>();
				data.respawnPoints.push_back(r);
			}
		}

		data.objects.clear();
		for (const auto& obj : j["objects"]) {
			data.objects.push_back({
				obj["type"].get<std::string>(),
				obj["x"].get<int>(),
				obj["y"].get<int>()
				});
		}

		data.transitions.clear();
		for (const auto& tp : j["transitions"]) {
			TransitionPoint t;
			t.id = tp["id"].get<std::string>();
			t.x = tp["x"].get<int>();
			t.y = tp["y"].get<int>();
			t.targetMap = tp["targetMap"].get<std::string>();
			t.targetPoint = tp["targetPoint"].get<std::string>();
			data.transitions.push_back(t);
		}

		return true;
	}

} // namespace Stage
