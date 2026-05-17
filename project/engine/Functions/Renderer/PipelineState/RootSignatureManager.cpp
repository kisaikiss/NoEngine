#include "stdafx.h"
#include "RootSignatureManager.h"

namespace NoEngine {
uint32_t RootSignatureManager::Register(const std::string& name, RootSignature rootSignature) {
	indexMap_[name] = static_cast<uint32_t>(rootSignatures_.size());
	rootSignatures_.emplace_back(std::move(rootSignature));
	return indexMap_[name];
}
RootSignature& RootSignatureManager::Get(const std::string& name) {
	return rootSignatures_[indexMap_[name]];
}
RootSignature& RootSignatureManager::Get(uint32_t index) {
	return rootSignatures_[index];
}
}