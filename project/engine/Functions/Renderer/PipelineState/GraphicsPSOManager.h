#pragma once
#include "engine/Runtime/PipelineStateObject/GraphicsPSO.h"

namespace NoEngine {

class GraphicsPSOManager {
public:
	uint32_t Register(const std::string& name, const GraphicsPSO& pso);
	GraphicsPSO& Get(const std::string& name);
	GraphicsPSO& Get(uint32_t index);
	uint32_t GetID(const std::string& name);
private:
	std::vector<GraphicsPSO> graphicsPSOs_;
	std::unordered_map<std::string, uint32_t> indexMap_;
};

}
