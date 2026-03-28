#include "stdafx.h"
#include "ClearOverConfigDataIO.h"
#include <fstream>
#include "externals/nlohmann/json.hpp"

ClearOverConfigComponent ClearOverConfigDataIO::Load(const std::string& path) {
	ClearOverConfigComponent config;
	std::ifstream ifs(path);
	if (!ifs.is_open()) {
		return config;
	}
	try {
		nlohmann::json j;
		ifs >> j;
		config = j.get<ClearOverConfigComponent>();
	} catch (...) {
		// パースエラーはデフォルト値で続行
	}
	return config;
}

void ClearOverConfigDataIO::Save(const ClearOverConfigComponent& config, const std::string& path) {
	nlohmann::json j = config;
	std::ofstream ofs(path);
	if (ofs.is_open()) {
		ofs << j.dump(2);
	}
}
