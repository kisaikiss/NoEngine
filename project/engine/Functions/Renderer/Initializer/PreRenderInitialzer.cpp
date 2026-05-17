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
		DXGI_FORMAT preRenderRTVFormat[] = { DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R10G10B10A2_UNORM, };

		defaultPSO.SetRenderTargetFormats(2, preRenderRTVFormat, DXGI_FORMAT_D24_UNORM_S8_UINT);
		defaultPSO.SetVertexShader(defaultVS.GetBytecode());
		defaultPSO.SetPixelShader(pixelShader.GetBytecode());
		defaultPSO.SetSampleMask(D3D12_DEFAULT_SAMPLE_MASK);
		defaultPSO.Finalize();
		ctx.RegisterGraphicsPSO(ConvertString(defaultPSOName), defaultPSO);
		ctx.RegisterRootSignature(ConvertString(defaultPSOName), std::move(defaultRootSignature));
	}

	// PreRenderPass Skining
	{
		ShaderModule defaultSkinnedVS(ShaderStage::Vertex, L"resources/engine/Shaders/DefaultSkinned.VS.hlsl", L"vs_6_0");
		ShaderModule pixelShader(ShaderStage::Pixel, L"resources/engine/Shaders/PreRender.PS.hlsl", L"ps_6_0");

		const ShaderReflection& prePSReflection = pixelShader.GetReflection();
		const ShaderReflection& vsSkinnedReflection = defaultSkinnedVS.GetReflection();

		std::vector<ShaderReflection> skinnedRefls;
		skinnedRefls.push_back(vsSkinnedReflection);
		skinnedRefls.push_back(prePSReflection);

		RootSignature defaultSkinnedRootSignature;
		std::wstring defaultSkinnedPSOName = L"Renderer : PreRenderSkinned PSO";
		RootSignatureBuilder::BuildFromReflection(skinnedRefls, defaultSkinnedRootSignature, ConvertString(defaultSkinnedPSOName));

		std::vector<D3D12_INPUT_ELEMENT_DESC> skinnedInputLayout = InputLayoutBuilder::BuildFromReflection(vsSkinnedReflection);
		// ToDo : inputLayoutのReflectionがUINT型のインプットが対応できていないので、このように後から入れる形になってしまっています。UINT型に対応すべきです。
		skinnedInputLayout[4].Format = DXGI_FORMAT_R32G32B32A32_UINT;


		GraphicsPSO defaultSkinnedPSO(defaultSkinnedPSOName);
		defaultSkinnedPSO.SetRootSignature(defaultSkinnedRootSignature);
		defaultSkinnedPSO.SetRasterizerState(rasterizerDesc);
		defaultSkinnedPSO.SetBlendState(blendDesc);
		defaultSkinnedPSO.SetDepthStencilState(depthStencilDesc);
		defaultSkinnedPSO.SetInputLayout(skinnedInputLayout);
		defaultSkinnedPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
		DXGI_FORMAT preRenderRTVFormat[] = { DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R10G10B10A2_UNORM, };

		defaultSkinnedPSO.SetRenderTargetFormats(2, preRenderRTVFormat, DXGI_FORMAT_D24_UNORM_S8_UINT);
		defaultSkinnedPSO.SetVertexShader(defaultSkinnedVS.GetBytecode());
		defaultSkinnedPSO.SetPixelShader(pixelShader.GetBytecode());
		defaultSkinnedPSO.SetSampleMask(D3D12_DEFAULT_SAMPLE_MASK);
		defaultSkinnedPSO.Finalize();
		ctx.RegisterGraphicsPSO(ConvertString(defaultSkinnedPSOName), defaultSkinnedPSO);
		ctx.RegisterRootSignature(ConvertString(defaultSkinnedPSOName), std::move(defaultSkinnedRootSignature));
	}
}
}