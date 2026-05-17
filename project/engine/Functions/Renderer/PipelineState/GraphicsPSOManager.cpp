#include "stdafx.h"
#include "GraphicsPSOManager.h"

namespace NoEngine {

uint32_t GraphicsPSOManager::Register(const std::string& name, const GraphicsPSO& pso) {
	indexMap_[name] = static_cast<uint32_t>(graphicsPSOs_.size());
	graphicsPSOs_.emplace_back(pso);
	return indexMap_[name];
}

GraphicsPSO& GraphicsPSOManager::Get(const std::string& name) {
	return graphicsPSOs_[indexMap_[name]];
}

GraphicsPSO& GraphicsPSOManager::Get(uint32_t index) {
	return graphicsPSOs_[index];
}

uint32_t GraphicsPSOManager::GetID(const std::string& name) {
	if (!indexMap_.contains(name)) return UINT32_MAX; // or 0xFFFFFFFF
	return indexMap_.at(name);
}

}