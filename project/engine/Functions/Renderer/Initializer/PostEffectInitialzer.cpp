#include "stdafx.h"
#include "PostEffectInitialzer.h"
#include "engine/Functions/Shader/ShaderModule.h"
#include "engine/Utilities/Conversion/ConvertString.h"

namespace NoEngine {
void PostEffectInitialzer::Initialize(RenderContext& ctx) {
	CreatePSO(ctx);
}
void PostEffectInitialzer::CreatePSO(RenderContext& ctx) {
	ShaderModule defaultVS(ShaderStage::Vertex, L"resources/engine/Shaders/FullScreen.VS.hlsl", L"vs_6_0");
	ShaderModule grayscalePS(ShaderStage::Pixel, L"resources/engine/Shaders/Grayscale.PS.hlsl", L"ps_6_0");

	const ShaderReflection& vsReflection = defaultVS.GetReflection();
	const ShaderReflection& psReflection = grayscalePS.GetReflection();
	std::vector<ShaderReflection> refls;
	refls.push_back(vsReflection);
	refls.push_back(psReflection);

	RootSignature defaultRootSignature;
	std::string defaultPSOName = "Renderer : Grayscale PSO";
	std::string rootSigName = "Renderer : PostEffectRootSig";
	RootSignatureBuilder::BuildFromReflection(refls, defaultRootSignature, rootSigName);
	
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	D3D12_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = false;

	GraphicsPSO defaultPSO(ConvertString(defaultPSOName));
	defaultPSO.SetRootSignature(defaultRootSignature);
	defaultPSO.SetRasterizerState(rasterizerDesc);
	defaultPSO.SetBlendState(blendDesc);
	defaultPSO.SetDepthStencilState(depthStencilDesc);
	defaultPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	DXGI_FORMAT rtvFormat[] = { DXGI_FORMAT_R8G8B8A8_UNORM_SRGB };
	defaultPSO.SetRenderTargetFormats(1, rtvFormat, DXGI_FORMAT_UNKNOWN);
	defaultPSO.SetVertexShader(defaultVS.GetBytecode());
	defaultPSO.SetPixelShader(grayscalePS.GetBytecode());
	defaultPSO.SetSampleMask(D3D12_DEFAULT_SAMPLE_MASK);
	defaultPSO.Finalize();

	ctx.RegisterGraphicsPSO(defaultPSOName, defaultPSO);
	ctx.RegisterRootSignature(rootSigName, std::move(defaultRootSignature));


	ShaderModule vignettingPS(ShaderStage::Pixel, L"resources/engine/Shaders/Vignetting.PS.hlsl", L"ps_6_0");
	defaultPSO.SetPixelShader(vignettingPS.GetBytecode());
	defaultPSO.Finalize();

	ctx.RegisterGraphicsPSO("Renderer : Vignetting PSO", defaultPSO);
}
}
