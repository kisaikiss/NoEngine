#include "stdafx.h"
#include "PreRenderInitialzer.h"
#include "engine/Functions/Shader/ShaderModule.h"
#include "engine/Utilities/Conversion/ConvertString.h"

namespace NoEngine {
void PreRenderInitialzer::Initialize(RenderContext& ctx) {
	CreatePSO(ctx);
}

void PreRenderInitialzer::CreatePSO(RenderContext& ctx) {
	ShaderModule defaultVS(ShaderStage::Vertex, L"resources/engine/Shaders/Default.VS.hlsl", L"vs_6_0");
	const ShaderReflection& vsReflection = defaultVS.GetReflection();

	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	D3D12_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = InputLayoutBuilder::BuildFromReflection(vsReflection);
	
	// PreRenderPass
	{
		ShaderModule pixelShader(ShaderStage::Pixel, L"resources/engine/Shaders/PreRender.PS.hlsl", L"ps_6_0");

		const ShaderReflection& prePSReflection = pixelShader.GetReflection();
		std::vector<ShaderReflection> refls;
		refls.push_back(vsReflection);
		refls.push_back(prePSReflection);

		RootSignature defaultRootSignature;
		std::wstring defaultPSOName = L"Renderer : PreRender PSO";
		RootSignatureBuilder::BuildFromReflection(refls, defaultRootSignature, ConvertString(defaultPSOName));

		GraphicsPSO defaultPSO(defaultPSOName);
		defaultPSO.SetRootSignature(defaultRootSignature);
		defaultPSO.SetRasterizerState(rasterizerDesc);
		defaultPSO.SetBlendState(blendDesc);
		defaultPSO.SetDepthStencilState(depthStencilDesc);
		defaultPSO.SetInputLayout(inputLayout);
		defaultPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
		DXGI_FORMAT preRenderRTVFormat[] = { DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R10G10B10A2_UNORM,DXGI_FORMAT_R8G8B8A8_UNORM };

		defaultPSO.SetRenderTargetFormats(3, preRenderRTVFormat, DXGI_FORMAT_D24_UNORM_S8_UINT);
		defaultPSO.SetVertexShader(defaultVS.GetBytecode());
		defaultPSO.SetPixelShader(pixelShader.GetBytecode());
		defaultPSO.SetSampleMask(D3D12_DEFAULT_SAMPLE_MASK);
		defaultPSO.Finalize();
		ctx.RegisterGraphicsPSO(ConvertString(defaultPSOName), defaultPSO);
		ctx.RegisterRootSignature(ConvertString(defaultPSOName), std::move(defaultRootSignature));
	}

	// 半透明オブジェクト用ObjectID
	{
		ShaderModule pixelShader(ShaderStage::Pixel, L"resources/engine/Shaders/ObjectID.PS.hlsl", L"ps_6_0");

		const ShaderReflection& prePSReflection = pixelShader.GetReflection();
		std::vector<ShaderReflection> refls;
		refls.push_back(vsReflection);
		refls.push_back(prePSReflection);

		RootSignature defaultRootSignature;
		std::wstring defaultPSOName = L"Renderer : Transparent PreRender PSO";
		RootSignatureBuilder::BuildFromReflection(refls, defaultRootSignature, ConvertString(defaultPSOName));

		GraphicsPSO defaultPSO(defaultPSOName);
		defaultPSO.SetRootSignature(defaultRootSignature);
		defaultPSO.SetRasterizerState(rasterizerDesc);
		defaultPSO.SetBlendState(blendDesc);
		D3D12_DEPTH_STENCIL_DESC idTransparentDepthDesc = depthStencilDesc;
		idTransparentDepthDesc.DepthEnable = false;
		idTransparentDepthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; // 半透明ID用: テストのみ
		defaultPSO.SetDepthStencilState(idTransparentDepthDesc);
		defaultPSO.SetInputLayout(inputLayout);
		defaultPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
		DXGI_FORMAT preRenderRTVFormat[] = { DXGI_FORMAT_R8G8B8A8_UNORM };

		defaultPSO.SetRenderTargetFormats(1, preRenderRTVFormat, DXGI_FORMAT_UNKNOWN);
		defaultPSO.SetVertexShader(defaultVS.GetBytecode());
		defaultPSO.SetPixelShader(pixelShader.GetBytecode());
		defaultPSO.SetSampleMask(D3D12_DEFAULT_SAMPLE_MASK);
		defaultPSO.Finalize();
		ctx.RegisterGraphicsPSO(ConvertString(defaultPSOName), defaultPSO);
		ctx.RegisterRootSignature(ConvertString(defaultPSOName), std::move(defaultRootSignature));
	}
}
}