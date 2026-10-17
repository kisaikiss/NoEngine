#pragma once
#include "engine/NoEngine.h"
class ShaderBackgroundPass : public No::RenderPass {
public:
	void Execute(NoEngine::GraphicsContext& gfx, const NoEngine::RenderGraphRegistry& resourceRegistry, No::Registry& registry) override;
private:
	struct Pipeline {
		Pipeline() : pso(L"ShaderBackground PSO") {}
		NoEngine::RootSignature rootSig;
		NoEngine::GraphicsPSO pso;
		std::string rootSigName;
	};
	Pipeline* GetOrCreate(const std::string& psPath);
	std::unordered_map<std::string, std::unique_ptr<Pipeline>> pipelines_; // nullptr = 失敗済み
};
