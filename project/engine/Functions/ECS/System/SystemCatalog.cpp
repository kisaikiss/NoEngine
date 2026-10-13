#include "stdafx.h"
#include "SystemCatalog.h"

namespace NoEngine {
namespace ECS {

SystemCatalog& SystemCatalog::Get() {
	// 静的初期化順序問題を避けるため関数内staticにする
	static SystemCatalog catalog;
	return catalog;
}

bool SystemCatalog::Register(const std::string& name, const std::string& category,
	std::type_index type, SystemFactory factory) {
	entries_[name] = Entry{ category, std::move(factory) };
	types_[type] = name;
	return true;
}

std::unique_ptr<ISystem> SystemCatalog::Create(const std::string& name) const {
	auto it = entries_.find(name);
	if (it == entries_.end()) return nullptr;
	return it->second.factory();
}

bool SystemCatalog::Contains(const std::string& name) const {
	return entries_.find(name) != entries_.end();
}

std::string SystemCatalog::NameOf(std::type_index type) const {
	auto it = types_.find(type);
	return it != types_.end() ? it->second : std::string();
}

std::map<std::string, std::vector<std::string>> SystemCatalog::GroupedByCategory() const {
	std::map<std::string, std::vector<std::string>> result;
	for (const auto& [name, entry] : entries_) {
		result[entry.category].push_back(name);
	}
	for (auto& [category, names] : result) {
		std::sort(names.begin(), names.end());
	}
	return result;
}

}
}