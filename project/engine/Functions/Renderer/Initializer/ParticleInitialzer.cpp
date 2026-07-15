#include "stdafx.h"
#include "ParticleInitialzer.h"
#include "engine/Functions/Shader/ShaderModule.h"
#include "engine/Utilities/Conversion/ConvertString.h"

namespace NoEngine {
void ParticleInitialzer::Initialize(RenderContext& ctx) {
	CreatePSO(ctx);
}

void ParticleInitialzer::CreatePSO(RenderContext& ctx) {
	DXGI_FORMAT rtvFormat[] = { DXGI_FORMAT_R8G8B8A8_UNORM_SRGB }; 
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	{
		ShaderModule particleVS(ShaderStage::Vertex, L"resources/engine/Shaders/Particle.VS.hlsl", L"vs_6_0");
		ShaderModule particlePS(ShaderStage::Pixel, L"resources/engine/Shaders/Particle.PS.hlsl", L"ps_6_0");

		const ShaderReflection& particleVsReflection = particleVS.GetReflection();
		const ShaderReflection& particlePsReflection = particlePS.GetReflection();

		std::vector<ShaderReflection> reflection;
		reflection.push_back(particleVsReflection);
		reflection.push_back(particlePsReflection);
		RootSignature particleRootSignature;
		std::wstring particlePSOName = L"Renderer : particle PSO";
		RootSignatureBuilder::BuildFromReflection(reflection, particleRootSignature, ConvertString(particlePSOName));

		D3D12_BLEND_DESC blendParticleDesc{};
		blendParticleDesc.IndependentBlendEnable = FALSE;
		blendParticleDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		blendParticleDesc.RenderTarget[0].BlendEnable = TRUE;
		blendParticleDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendParticleDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		blendParticleDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendParticleDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		blendParticleDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
		blendParticleDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;

		D3D12_DEPTH_STENCIL_DESC depthStencilParticleDesc{};
		depthStencilParticleDesc.DepthEnable = true;
		depthStencilParticleDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
		depthStencilParticleDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

		std::vector<D3D12_INPUT_ELEMENT_DESC> particleInputLayout = InputLayoutBuilder::BuildFromReflection(particleVsReflection);
		// ShaderReflectionがInstanceIDを認識してしまうので削除する
		particleInputLayout.erase(particleInputLayout.end() - 1);

		GraphicsPSO particlePSO(particlePSOName);

		particlePSO.SetRootSignature(particleRootSignature);
		particlePSO.SetRasterizerState(rasterizerDesc);
		particlePSO.SetBlendState(blendParticleDesc);
		particlePSO.SetDepthStencilState(depthStencilParticleDesc);
		particlePSO.SetInputLayout(particleInputLayout);
		particlePSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
		particlePSO.SetRenderTargetFormats(1, rtvFormat, DXGI_FORMAT_D24_UNORM_S8_UINT);
		particlePSO.SetVertexShader(particleVS.GetBytecode());
		particlePSO.SetPixelShader(particlePS.GetBytecode());
		particlePSO.SetSampleMask(D3D12_DEFAULT_SAMPLE_MASK);
		particlePSO.Finalize();
		ctx.RegisterGraphicsPSO(ConvertString(particlePSOName), particlePSO);
		ctx.RegisterRootSignature(ConvertString(particlePSOName), std::move(particleRootSignature));
	}
}
}