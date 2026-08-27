#include "stdafx.h"
#include "TextInitializer.h"
#include "engine/Functions/Shader/ShaderModule.h"
#include "engine/Utilities/Conversion/ConvertString.h"
namespace NoEngine {
void TextInitializer::Initialize(RenderContext& ctx) {
	CreatePSO(ctx);
}

void TextInitializer::CreatePSO(RenderContext& ctx) {
	DXGI_FORMAT rtvFormat[] = { DXGI_FORMAT_R8G8B8A8_UNORM_SRGB };
	ShaderModule textVS(ShaderStage::Vertex, L"resources/engine/Shaders/Text.VS.hlsl", L"vs_6_0");
	ShaderModule textPS(ShaderStage::Pixel, L"resources/engine/Shaders/Text.PS.hlsl", L"ps_6_0");

	const ShaderReflection& textVsReflection = textVS.GetReflection();
	const ShaderReflection& textPsReflection = textPS.GetReflection();

	std::vector<ShaderReflection> reflectionText;
	reflectionText.push_back(textVsReflection);
	reflectionText.push_back(textPsReflection);
	RootSignature textRootSignature;
	std::wstring textPSOName = L"Renderer : Text PSO";
	RootSignatureBuilder::BuildFromReflection(reflectionText, textRootSignature, ConvertString(textPSOName));

	D3D12_RASTERIZER_DESC rasterizerTextDesc{};
	rasterizerTextDesc.CullMode = D3D12_CULL_MODE_NONE;
	rasterizerTextDesc.FillMode = D3D12_FILL_MODE_SOLID;

	D3D12_BLEND_DESC blendTextDesc{};
	blendTextDesc.IndependentBlendEnable = FALSE;
	blendTextDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendTextDesc.RenderTarget[0].BlendEnable = TRUE;
	blendTextDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendTextDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendTextDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendTextDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendTextDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
	blendTextDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;

	D3D12_DEPTH_STENCIL_DESC depthStencilTextDesc{};
	depthStencilTextDesc.DepthEnable = false;
	depthStencilTextDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilTextDesc.DepthFunc = D3D12_COMPARISON_FUNC_NONE;

	std::vector<D3D12_INPUT_ELEMENT_DESC> textInputLayout = InputLayoutBuilder::BuildFromReflection(textVsReflection);

	GraphicsPSO textPSO(textPSOName);
	textPSO.SetRootSignature(textRootSignature);
	textPSO.SetRasterizerState(rasterizerTextDesc);
	textPSO.SetBlendState(blendTextDesc);
	textPSO.SetDepthStencilState(depthStencilTextDesc);
	textPSO.SetInputLayout(textInputLayout);
	textPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	textPSO.SetRenderTargetFormats(1, rtvFormat, DXGI_FORMAT_UNKNOWN);
	textPSO.SetVertexShader(textVS.GetBytecode());
	textPSO.SetPixelShader(textPS.GetBytecode());
	textPSO.SetSampleMask(D3D12_DEFAULT_SAMPLE_MASK);
	textPSO.Finalize();
	ctx.RegisterGraphicsPSO(ConvertString(textPSOName), textPSO);
	ctx.RegisterRootSignature(ConvertString(textPSOName), std::move(textRootSignature));
}
}