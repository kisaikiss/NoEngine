#pragma once
#include "engine/Runtime/PipelineStateObject/RootSignature.h"
#include "engine/Utilities/NonCopyable.h"

namespace NoEngine {

class RootSignatureManager : NonCopyable {
public:
	RootSignatureManager() = default;

	RootSignatureManager(const RootSignatureManager&) = delete;
	RootSignatureManager& operator=(const RootSignatureManager&) = delete;

	RootSignatureManager(RootSignatureManager&&) = default;
	RootSignatureManager& operator=(RootSignatureManager&&) = default;
	uint32_t Register(const std::string& name, RootSignature rootSignature);
	RootSignature& Get(const std::string& name);
	RootSignature& Get(uint32_t index);
	uint32_t GetID(const std::string& name) {
		if (!indexMap_.contains(name)) return UINT32_MAX; // or 0xFFFFFFFF
		return indexMap_.at(name);
	}
private:
	std::vector<RootSignature> rootSignatures_;
	std::unordered_map<std::string, uint32_t> indexMap_;
};

}

