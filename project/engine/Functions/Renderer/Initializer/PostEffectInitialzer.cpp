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
	std::vector<ShaderReflection> defaultReflections;
	defaultReflections.push_back(vsReflection);
	defaultReflections.push_back(psReflection);

	RootSignature defaultRootSignature;
	std::string defaultPSOName = "Renderer : Grayscale PSO";
	std::string rootSigName = "Renderer : PostEffectRootSig";
	RootSignatureBuilder::BuildFromReflection(defaultReflections, defaultRootSignature, rootSigName);
	
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

	ShaderModule GaussianPS(ShaderStage::Pixel, L"resources/engine/Shaders/GaussianFilter.PS.hlsl", L"ps_6_0");
	defaultPSO.SetPixelShader(GaussianPS.GetBytecode());
	defaultPSO.Finalize();

	ctx.RegisterGraphicsPSO("Renderer : Gaussian PSO", defaultPSO);

	ShaderModule radialPS(ShaderStage::Pixel, L"resources/engine/Shaders/RadialBlur.PS.hlsl", L"ps_6_0");
	defaultPSO.SetPixelShader(radialPS.GetBytecode());
	defaultPSO.Finalize();
	ctx.RegisterGraphicsPSO("Renderer : RadialBlur PSO", defaultPSO);

	

	{
		ShaderModule boxFilterPS(ShaderStage::Pixel, L"resources/engine/Shaders/BoxFilter.PS.hlsl", L"ps_6_0");

		const ShaderReflection& boxFilterPSReflection = boxFilterPS.GetReflection();
		std::vector<ShaderReflection> boxFilterReflections;
		boxFilterReflections.push_back(vsReflection);
		boxFilterReflections.push_back(boxFilterPSReflection);

		RootSignature boxFilterRootSignature;
		RootSignatureBuilder::BuildFromReflection(boxFilterReflections, boxFilterRootSignature, "Renderer : BoxFilter");

		defaultPSO.SetPixelShader(boxFilterPS.GetBytecode());
		defaultPSO.SetRootSignature(boxFilterRootSignature);
		defaultPSO.Finalize();

		ctx.RegisterGraphicsPSO("Renderer : BoxFilter PSO", defaultPSO);
		ctx.RegisterRootSignature("Renderer : BoxFilter", std::move(boxFilterRootSignature));
	}

	{
		ShaderModule outlinePS(ShaderStage::Pixel, L"resources/engine/Shaders/DepthBasedOutline.PS.hlsl", L"ps_6_0");

		const ShaderReflection& outlinePSReflection = outlinePS.GetReflection();
		std::vector<ShaderReflection> outlineReflections;
		outlineReflections.push_back(vsReflection);
		outlineReflections.push_back(outlinePSReflection);

		RootSignature outlineRootSignature;
		RootSignatureBuilder::BuildFromReflection(outlineReflections, outlineRootSignature, "Renderer : Outline");

		defaultPSO.SetPixelShader(outlinePS.GetBytecode());
		defaultPSO.SetRootSignature(outlineRootSignature);
		defaultPSO.Finalize();

		ctx.RegisterGraphicsPSO("Renderer : Outline PSO", defaultPSO);
		ctx.RegisterRootSignature("Renderer : Outline", std::move(outlineRootSignature));
	}

	{
		ShaderModule pixelShader(ShaderStage::Pixel, L"resources/engine/Shaders/Dissolve.PS.hlsl", L"ps_6_0");

		const ShaderReflection& pixelShaderReflection = pixelShader.GetReflection();
		std::vector<ShaderReflection> reflections;
		reflections.push_back(vsReflection);
		reflections.push_back(pixelShaderReflection);

		RootSignature rootSignature;
		RootSignatureBuilder::BuildFromReflection(reflections, rootSignature, "Renderer : Dissolve");

		defaultPSO.SetPixelShader(pixelShader.GetBytecode());
		defaultPSO.SetRootSignature(rootSignature);
		defaultPSO.Finalize();

		ctx.RegisterGraphicsPSO("Renderer : Dissolve PSO", defaultPSO);
		ctx.RegisterRootSignature("Renderer : Dissolve", std::move(rootSignature));
	}

	{
		ShaderModule pixelShader(ShaderStage::Pixel, L"resources/engine/Shaders/RandomNoise.PS.hlsl", L"ps_6_0");
		const ShaderReflection& pixelShaderReflection = pixelShader.GetReflection();
		std::vector<ShaderReflection> reflections;
		reflections.push_back(vsReflection);
		reflections.push_back(pixelShaderReflection);

		RootSignature rootSignature;
		RootSignatureBuilder::BuildFromReflection(reflections, rootSignature, "Renderer : RandomNoise");

		defaultPSO.SetPixelShader(pixelShader.GetBytecode());
		defaultPSO.SetRootSignature(rootSignature);
		defaultPSO.Finalize();
		ctx.RegisterGraphicsPSO("Renderer : RandomNoise PSO", defaultPSO);
		ctx.RegisterRootSignature("Renderer : RandomNoise", std::move(rootSignature));
	}
}
}
