#include "BackGroundEffect.h"
#include "engine/Functions/Shader/ShaderModule.h"
#include "engine/Utilities/Conversion/ConvertString.h"
#include "engine/Functions/ECS/Component/TransformComponent.h"
#include "../Component/BackGroundComponent.h"
#include "engine/Math/Color/Color.h"

using namespace NoEngine;

BackGroundEffectPass::BackGroundEffectPass()
{
	ShaderModule primitiveVS(ShaderStage::Vertex, L"resources/game/Shaders/BackGround.VS.hlsl", L"vs_6_0");
	ShaderModule primitivePS(ShaderStage::Pixel, L"resources/game/Shaders/BackGround.PS.hlsl", L"ps_6_0");

	const ShaderReflection& PrimitiveVsReflection = primitiveVS.GetReflection();
	const ShaderReflection& PrimitivePsReflection = primitivePS.GetReflection();

	std::vector<ShaderReflection> primitiveRefls;
	primitiveRefls.push_back(PrimitiveVsReflection);
	primitiveRefls.push_back(PrimitivePsReflection);

	std::wstring psoName = L"BackGround PSO";
	RootSignatureBuilder::BuildFromReflection(primitiveRefls, rootSignature_, ConvertString(psoName));

	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	D3D12_BLEND_DESC blendDesc = {};
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	std::vector<D3D12_INPUT_ELEMENT_DESC> primitiveInputLayout = InputLayoutBuilder::BuildFromReflection(PrimitiveVsReflection);

	pso_ = GraphicsPSO(psoName);

	pso_.SetRootSignature(rootSignature_);
	pso_.SetRasterizerState(rasterizerDesc);
	pso_.SetBlendState(blendDesc);
	pso_.SetDepthStencilState(depthStencilDesc);
	pso_.SetInputLayout(primitiveInputLayout);
	pso_.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	pso_.SetRenderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, DXGI_FORMAT_D24_UNORM_S8_UINT);
	pso_.SetVertexShader(primitiveVS.GetBytecode());
	pso_.SetPixelShader(primitivePS.GetBytecode());
	pso_.SetSampleMask(D3D12_DEFAULT_SAMPLE_MASK);
	pso_.Finalize();

	meshData_.vertices = {
				{ Vector3{ -0.5f, -0.5f, 0.0f }, Vector2{ 0.0f, 1.0f } },
				{ Vector3{ -0.5f,  0.5f, 0.0f }, Vector2{ 0.0f, 0.0f } },
				{ Vector3{  0.5f, -0.5f, 0.0f }, Vector2{ 1.0f, 1.0f } },
				{ Vector3{ -0.5f,  0.5f, 0.0f }, Vector2{ 0.0f, 0.0f } },

				{ Vector3{  0.5f,  0.5f, 0.0f }, Vector2{ 1.0f, 0.0f } },
				{ Vector3{  0.5f, -0.5f, 0.0f }, Vector2{ 1.0f, 1.0f } }
	};

}

BackGroundEffectPass::~BackGroundEffectPass()
{

}

void BackGroundEffectPass::Execute(NoEngine::GraphicsContext& gfx, NoEngine::ECS::Registry& registry)
{
	auto view = registry.View<Component::TransformComponent, BackGroundComponent>();
	if (view.Empty()) return;
	gfx.SetRootSignature(rootSignature_);
	gfx.SetPipelineState(pso_);
	gfx.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (auto entity : view)
	{
		auto* transform = registry.GetComponent<Component::TransformComponent>(entity);
		auto* backGround = registry.GetComponent<BackGroundComponent>(entity);

		auto& rootIndex = RootSignatureBuilder::GetRootIndexMap("BackGround PSO");

		struct VSConstants
		{
			Matrix4x4 WorldMat;
			Matrix4x4 UVTransform;
			Matrix4x4 ViewProjMat;
		} vsConstants;
		vsConstants.WorldMat = transform->MakeAffineMatrix4x4();
		vsConstants.UVTransform = backGround->uvTransform;
		vsConstants.ViewProjMat = GetCamera()->GetViewProjMatrix();

		_declspec(align(16)) struct PSConstants
		{
			Color bgColor;
			Color ringColor;
			float time;
			float timeScale;
			float powerFactor;
			float fadeInner;
			float fadeOuter;
			float seed;
			float variant;
			uint32_t useRing;
		} psConstants;
		psConstants.bgColor = backGround->bgColor;
		psConstants.ringColor = backGround->ringColor;
		psConstants.time = backGround->time;
		psConstants.timeScale = backGround->timeScale;
		psConstants.powerFactor = backGround->powerFactor;
		psConstants.fadeInner = backGround->fadeInner;
		psConstants.fadeOuter = backGround->fadeOuter;
		psConstants.seed = backGround->seed;
		psConstants.variant = backGround->variant;
		psConstants.useRing = backGround->useRing;

		gfx.SetDynamicVB(0, meshData_.vertices.size(), sizeof(vertex), meshData_.vertices.data());
		gfx.SetDynamicConstantBufferView(rootIndex["gWorld"], sizeof(vsConstants), &vsConstants);
		gfx.SetDynamicConstantBufferView(rootIndex["gControl"], sizeof(psConstants), &psConstants);

		gfx.Draw(static_cast<UINT>(meshData_.vertices.size()), 0);
	}
	
}
