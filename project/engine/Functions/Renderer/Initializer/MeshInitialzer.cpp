#include "stdafx.h"
#include "MeshInitialzer.h"
#include "engine/Functions/Shader/ShaderModule.h"
#include "engine/Utilities/Conversion/ConvertString.h"


namespace NoEngine {

void MeshInitialzer::Initialize(RenderContext& ctx) {
    CreatePSO(ctx);

}

void MeshInitialzer::CreatePSO(RenderContext& ctx) {
    ShaderModule defaultVS(ShaderStage::Vertex, L"resources/engine/Shaders/Default.VS.hlsl", L"vs_6_0");
    ShaderModule defaultPS(ShaderStage::Pixel, L"resources/engine/Shaders/Default.PS.hlsl", L"ps_6_0");
	const ShaderReflection& vsReflection = defaultVS.GetReflection();
	const ShaderReflection& psReflection = defaultPS.GetReflection();

	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	D3D12_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	D3D12_BLEND_DESC addBlendDesc{};
	addBlendDesc.RenderTarget[0].BlendEnable = TRUE;
	addBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	addBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	addBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
	addBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	addBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	addBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	addBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	D3D12_BLEND_DESC subBlendDesc = addBlendDesc;
	subBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_SUBTRACT;

	D3D12_BLEND_DESC mulBlendDesc{};
	mulBlendDesc.RenderTarget[0].BlendEnable = TRUE;
	mulBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
	mulBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	mulBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_SRC_COLOR;
	mulBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	mulBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	mulBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	mulBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	D3D12_BLEND_DESC screenBlendDesc{};
	screenBlendDesc.RenderTarget[0].BlendEnable = TRUE;
	screenBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_INV_DEST_COLOR;
	screenBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	screenBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
	screenBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	screenBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	screenBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	screenBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	DXGI_FORMAT rtvFormat[] = { DXGI_FORMAT_R8G8B8A8_UNORM_SRGB };


	D3D12_DEPTH_STENCIL_DESC transparentDepthStencilDesc = depthStencilDesc;
	transparentDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = InputLayoutBuilder::BuildFromReflection(vsReflection);
	// デフォルト描画
	{
		std::vector<ShaderReflection> refls;
		refls.push_back(vsReflection);
		refls.push_back(psReflection);

		RootSignature defaultRootSignature;
		std::string defaultPSOName = "Renderer : Default PSO";
		RootSignatureBuilder::BuildFromReflection(refls, defaultRootSignature, defaultPSOName);

		GraphicsPSO defaultPSO(ConvertString(defaultPSOName));
		defaultPSO.SetRootSignature(defaultRootSignature);
		defaultPSO.SetRasterizerState(rasterizerDesc);
		defaultPSO.SetBlendState(blendDesc);
		defaultPSO.SetDepthStencilState(depthStencilDesc);
		defaultPSO.SetInputLayout(inputLayout);
		defaultPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

		defaultPSO.SetRenderTargetFormats(1, rtvFormat, DXGI_FORMAT_D24_UNORM_S8_UINT);
		defaultPSO.SetVertexShader(defaultVS.GetBytecode());
		defaultPSO.SetPixelShader(defaultPS.GetBytecode());
		defaultPSO.SetSampleMask(D3D12_DEFAULT_SAMPLE_MASK);
		defaultPSO.Finalize();
		
		ctx.RegisterGraphicsPSO(defaultPSOName, defaultPSO);
		// 加算合成
		defaultPSOName = "Renderer : Default Add PSO";
		defaultPSO.SetBlendState(addBlendDesc);
		defaultPSO.SetDepthStencilState(transparentDepthStencilDesc);
		defaultPSO.Finalize();
		ctx.RegisterGraphicsPSO(defaultPSOName, defaultPSO);

		// 減算合成
		defaultPSOName = "Renderer : Default Sub PSO";
		defaultPSO.SetBlendState(subBlendDesc);
		defaultPSO.Finalize();
		ctx.RegisterGraphicsPSO(defaultPSOName, defaultPSO);

		// 乗算合成
		defaultPSOName = "Renderer : Default Mul PSO";
		defaultPSO.SetBlendState(mulBlendDesc);
		defaultPSO.Finalize();
		ctx.RegisterGraphicsPSO(defaultPSOName, defaultPSO);

		// スクリーン合成
		defaultPSOName = "Renderer : Default Screen PSO";
		defaultPSO.SetBlendState(screenBlendDesc);
		defaultPSO.Finalize();
		ctx.RegisterGraphicsPSO(defaultPSOName, defaultPSO);

		ctx.RegisterRootSignature("Renderer : Default PSO", std::move(defaultRootSignature));
	}
	

	// トゥーンレンダリング
	ShaderModule ToonPS(ShaderStage::Pixel, L"resources/engine/Shaders/Toon.PS.hlsl", L"ps_6_0");
	const ShaderReflection& ToonReflection = ToonPS.GetReflection();
	{
		std::vector<ShaderReflection> reflectionToon;
		reflectionToon.push_back(vsReflection);
		reflectionToon.push_back(ToonReflection);
		RootSignature toonRootSignature;
		std::string toonPSOName = "Renderer : Toon PSO";
		RootSignatureBuilder::BuildFromReflection(reflectionToon, toonRootSignature, toonPSOName);

		GraphicsPSO toonPSO(ConvertString(toonPSOName));

		toonPSO.SetRootSignature(toonRootSignature);
		toonPSO.SetRasterizerState(rasterizerDesc);
		toonPSO.SetBlendState(blendDesc);
		toonPSO.SetDepthStencilState(depthStencilDesc);
		toonPSO.SetInputLayout(inputLayout);
		toonPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
		toonPSO.SetRenderTargetFormats(1, rtvFormat, DXGI_FORMAT_D24_UNORM_S8_UINT);
		toonPSO.SetVertexShader(defaultVS.GetBytecode());
		toonPSO.SetPixelShader(ToonPS.GetBytecode());
		toonPSO.SetSampleMask(D3D12_DEFAULT_SAMPLE_MASK);
		toonPSO.Finalize();
		ctx.RegisterGraphicsPSO(toonPSOName, toonPSO);

		// 加算合成
		toonPSOName = "Renderer : Toon Add PSO";
		toonPSO.SetBlendState(addBlendDesc);
		toonPSO.SetDepthStencilState(transparentDepthStencilDesc);
		toonPSO.Finalize();
		ctx.RegisterGraphicsPSO(toonPSOName, toonPSO);

		// 減算合成
		toonPSOName = "Renderer : Toon Sub PSO";
		toonPSO.SetBlendState(subBlendDesc);
		toonPSO.Finalize();
		ctx.RegisterGraphicsPSO(toonPSOName, toonPSO);

		// 乗算合成
		toonPSOName = "Renderer : Toon Mul PSO";
		toonPSO.SetBlendState(mulBlendDesc);
		toonPSO.Finalize();
		ctx.RegisterGraphicsPSO(toonPSOName, toonPSO);

		// スクリーン合成
		toonPSOName = "Renderer : Toon Screen PSO";
		toonPSO.SetBlendState(screenBlendDesc);
		toonPSO.Finalize();
		ctx.RegisterGraphicsPSO(toonPSOName, toonPSO);

		ctx.RegisterRootSignature("Renderer : Toon PSO", std::move(toonRootSignature));
	}

	// 発光オブジェクト
	ShaderModule emissivePS(ShaderStage::Pixel, L"resources/engine/Shaders/Emissive.PS.hlsl", L"ps_6_0");
	const ShaderReflection& emissiveReflection = emissivePS.GetReflection();
	{
		std::vector<ShaderReflection> refls;
		refls.push_back(vsReflection);
		refls.push_back(emissiveReflection);

		RootSignature emissiveRootSignature;
		std::string emissivePSOName = "Renderer : Emissive PSO";
		RootSignatureBuilder::BuildFromReflection(refls, emissiveRootSignature, emissivePSOName);

		GraphicsPSO emissivePSO(ConvertString(emissivePSOName));
		emissivePSO.SetRootSignature(emissiveRootSignature);
		D3D12_RASTERIZER_DESC emissiveRasterizerDesc = rasterizerDesc;
		emissiveRasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
		emissivePSO.SetRasterizerState(emissiveRasterizerDesc);
		emissivePSO.SetInputLayout(inputLayout);
		emissivePSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
		emissivePSO.SetBlendState(addBlendDesc);
		emissivePSO.SetDepthStencilState(transparentDepthStencilDesc);
		emissivePSO.SetRenderTargetFormats(1, rtvFormat, DXGI_FORMAT_D24_UNORM_S8_UINT);
		emissivePSO.SetVertexShader(defaultVS.GetBytecode());
		emissivePSO.SetPixelShader(emissivePS.GetBytecode());
		emissivePSO.SetSampleMask(D3D12_DEFAULT_SAMPLE_MASK);
		emissivePSO.Finalize();
		ctx.RegisterGraphicsPSO(emissivePSOName, emissivePSO);
		ctx.RegisterRootSignature(emissivePSOName, std::move(emissiveRootSignature));
	}

	// アウトライン
	{
		ShaderModule outlineVS(ShaderStage::Vertex, L"resources/engine/Shaders/Outline.VS.hlsl", L"vs_6_0");
		ShaderModule blackPS(ShaderStage::Pixel, L"resources/engine/Shaders/Black.PS.hlsl", L"ps_6_0");

		const ShaderReflection& outlineVsReflection = outlineVS.GetReflection();
		const ShaderReflection& blackPsReflection = blackPS.GetReflection();

		std::vector<ShaderReflection> reflectionOutline;
		reflectionOutline.push_back(outlineVsReflection);
		reflectionOutline.push_back(blackPsReflection);
		RootSignature outlineRootSignature;
		std::string outlinePSOName = "Renderer : outline PSO";
		RootSignatureBuilder::BuildFromReflection(reflectionOutline, outlineRootSignature, outlinePSOName);

		D3D12_RASTERIZER_DESC rasterizerOutlineDesc{};
		rasterizerOutlineDesc.CullMode = D3D12_CULL_MODE_FRONT;
		rasterizerOutlineDesc.FillMode = D3D12_FILL_MODE_SOLID;

		std::vector<D3D12_INPUT_ELEMENT_DESC> outlineInputLayout = InputLayoutBuilder::BuildFromReflection(outlineVsReflection);

		GraphicsPSO outlinePSO(ConvertString(outlinePSOName));

		outlinePSO.SetRootSignature(outlineRootSignature);
		outlinePSO.SetRasterizerState(rasterizerOutlineDesc);
		outlinePSO.SetBlendState(blendDesc);
		outlinePSO.SetDepthStencilState(depthStencilDesc);
		outlinePSO.SetInputLayout(outlineInputLayout);
		outlinePSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
		outlinePSO.SetRenderTargetFormats(1, rtvFormat, DXGI_FORMAT_D24_UNORM_S8_UINT);
		outlinePSO.SetVertexShader(outlineVS.GetBytecode());
		outlinePSO.SetPixelShader(blackPS.GetBytecode());
		outlinePSO.SetSampleMask(D3D12_DEFAULT_SAMPLE_MASK);
		outlinePSO.Finalize();
		ctx.RegisterGraphicsPSO(outlinePSOName, outlinePSO);
		ctx.RegisterRootSignature(outlinePSOName, std::move(outlineRootSignature));
	}
}

}