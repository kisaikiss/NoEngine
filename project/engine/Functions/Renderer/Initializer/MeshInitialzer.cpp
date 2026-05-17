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

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	DXGI_FORMAT rtvFormat[] = { DXGI_FORMAT_R8G8B8A8_UNORM_SRGB };

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
		ctx.RegisterRootSignature(defaultPSOName, std::move(defaultRootSignature));
	}

	// スキニング
	{
		ShaderModule defaultSkinnedVS(ShaderStage::Vertex, L"resources/engine/Shaders/DefaultSkinned.VS.hlsl", L"vs_6_0");

		const ShaderReflection& vsSkinnedReflection = defaultSkinnedVS.GetReflection();
		std::vector<ShaderReflection> skinnedRefls;
		skinnedRefls.push_back(vsSkinnedReflection);
		skinnedRefls.push_back(psReflection);

		RootSignature defaultSkinnedRootSignature;
		std::string defaultSkinnedPSOName = "Renderer : DefaultSkinned PSO";
		RootSignatureBuilder::BuildFromReflection(skinnedRefls, defaultSkinnedRootSignature, defaultSkinnedPSOName);

		std::vector<D3D12_INPUT_ELEMENT_DESC> skinnedInputLayout = InputLayoutBuilder::BuildFromReflection(vsSkinnedReflection);
		// ToDo : inputLayoutのReflectionがUINT型のインプットが対応できていないので、このように後から入れる形になってしまっています。UINT型に対応すべきです。
		skinnedInputLayout[4].Format = DXGI_FORMAT_R32G32B32A32_UINT;


		GraphicsPSO defaultSkinnedPSO(ConvertString(defaultSkinnedPSOName));
		defaultSkinnedPSO.SetRootSignature(defaultSkinnedRootSignature);
		defaultSkinnedPSO.SetRasterizerState(rasterizerDesc);
		defaultSkinnedPSO.SetBlendState(blendDesc);
		defaultSkinnedPSO.SetDepthStencilState(depthStencilDesc);
		defaultSkinnedPSO.SetInputLayout(skinnedInputLayout);
		defaultSkinnedPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
		defaultSkinnedPSO.SetRenderTargetFormats(1, rtvFormat, DXGI_FORMAT_D24_UNORM_S8_UINT);
		defaultSkinnedPSO.SetVertexShader(defaultSkinnedVS.GetBytecode());
		defaultSkinnedPSO.SetPixelShader(defaultPS.GetBytecode());
		defaultSkinnedPSO.SetSampleMask(D3D12_DEFAULT_SAMPLE_MASK);
		defaultSkinnedPSO.Finalize();
		ctx.RegisterGraphicsPSO(defaultSkinnedPSOName, defaultSkinnedPSO);
		ctx.RegisterRootSignature(defaultSkinnedPSOName, std::move(defaultSkinnedRootSignature));
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
		ctx.RegisterRootSignature(toonPSOName, std::move(toonRootSignature));
	}

	// スキニングトゥーンレンダリング
	{
		ShaderModule defaultSkinnedVS(ShaderStage::Vertex, L"resources/engine/Shaders/DefaultSkinned.VS.hlsl", L"vs_6_0");

		const ShaderReflection& vsSkinnedReflection = defaultSkinnedVS.GetReflection();
		std::vector<ShaderReflection> skinnedRefls;
		skinnedRefls.push_back(vsSkinnedReflection);
		skinnedRefls.push_back(ToonReflection);

		RootSignature defaultSkinnedRootSignature;
		std::string defaultSkinnedPSOName = "Renderer : ToonSkinned PSO";
		RootSignatureBuilder::BuildFromReflection(skinnedRefls, defaultSkinnedRootSignature, defaultSkinnedPSOName);

		std::vector<D3D12_INPUT_ELEMENT_DESC> skinnedInputLayout = InputLayoutBuilder::BuildFromReflection(vsSkinnedReflection);
		// ToDo : inputLayoutのReflectionがUINT型のインプットが対応できていないので、このように後から入れる形になってしまっています。UINT型に対応すべきです。
		skinnedInputLayout[4].Format = DXGI_FORMAT_R32G32B32A32_UINT;


		GraphicsPSO defaultSkinnedPSO(ConvertString(defaultSkinnedPSOName));
		defaultSkinnedPSO.SetRootSignature(defaultSkinnedRootSignature);
		defaultSkinnedPSO.SetRasterizerState(rasterizerDesc);
		defaultSkinnedPSO.SetBlendState(blendDesc);
		defaultSkinnedPSO.SetDepthStencilState(depthStencilDesc);
		defaultSkinnedPSO.SetInputLayout(skinnedInputLayout);
		defaultSkinnedPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
		defaultSkinnedPSO.SetRenderTargetFormats(1, rtvFormat, DXGI_FORMAT_D24_UNORM_S8_UINT);
		defaultSkinnedPSO.SetVertexShader(defaultSkinnedVS.GetBytecode());
		defaultSkinnedPSO.SetPixelShader(ToonPS.GetBytecode());
		defaultSkinnedPSO.SetSampleMask(D3D12_DEFAULT_SAMPLE_MASK);
		defaultSkinnedPSO.Finalize();
		ctx.RegisterGraphicsPSO(defaultSkinnedPSOName, defaultSkinnedPSO);
		ctx.RegisterRootSignature(defaultSkinnedPSOName, std::move(defaultSkinnedRootSignature));
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

	// スキニングするモデルのアウトライン
	{
		ShaderModule outlineVS(ShaderStage::Vertex, L"resources/engine/Shaders/OutlineSkinned.VS.hlsl", L"vs_6_0");
		ShaderModule blackPS(ShaderStage::Pixel, L"resources/engine/Shaders/Black.PS.hlsl", L"ps_6_0");

		const ShaderReflection& outlineVsReflection = outlineVS.GetReflection();
		const ShaderReflection& blackPsReflection = blackPS.GetReflection();

		std::vector<ShaderReflection> reflectionOutline;
		reflectionOutline.push_back(outlineVsReflection);
		reflectionOutline.push_back(blackPsReflection);
		RootSignature outlineRootSignature;
		std::string outlinePSOName = "Renderer : skinnedOutline PSO";
		RootSignatureBuilder::BuildFromReflection(reflectionOutline, outlineRootSignature, outlinePSOName);

		D3D12_RASTERIZER_DESC rasterizerOutlineDesc{};
		rasterizerOutlineDesc.CullMode = D3D12_CULL_MODE_FRONT;
		rasterizerOutlineDesc.FillMode = D3D12_FILL_MODE_SOLID;

		std::vector<D3D12_INPUT_ELEMENT_DESC> outlineInputLayout = InputLayoutBuilder::BuildFromReflection(outlineVsReflection);
		outlineInputLayout[4].Format = DXGI_FORMAT_R32G32B32A32_UINT;

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