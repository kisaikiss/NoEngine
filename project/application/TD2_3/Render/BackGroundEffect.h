#pragma once
#include "engine/Functions/Renderer/RenderPass/RenderPass.h"
#include "engine/Runtime/PipelineStateObject/GraphicsPSO.h"

class BackGroundEffectPass : public NoEngine::Render::RenderPass
{
public:
	BackGroundEffectPass();
	~BackGroundEffectPass();
	virtual void Execute(NoEngine::GraphicsContext& gfx, NoEngine::ECS::Registry& registry) override;

private:
	NoEngine::RootSignature rootSignature_;
	NoEngine::GraphicsPSO pso_;
	struct vertex
	{
		NoEngine::Vector3 position;
		NoEngine::Vector2 uv;
	};
	struct MeshData
	{
		std::vector<vertex> vertices;
		std::vector<uint16_t> indices;
	};
	MeshData meshData_;
};