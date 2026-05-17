#include "stdafx.h"
#include "PrimitiveInitializer.h"
#include "engine/Functions/Shader/ShaderModule.h"
#include "engine/Utilities/Conversion/ConvertString.h"

namespace NoEngine {
void PrimitiveInitializer::Initialize(RenderContext& ctx) { CreatePSO(ctx); }

void PrimitiveInitializer::CreatePSO(RenderContext& ctx) {
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	DXGI_FORMAT rtvFormat[] = { DXGI_FORMAT_R8G8B8A8_UNORM_SRGB };
	// debugPrimitive
	{
		ShaderModule primitiveVS(ShaderStage::Vertex, L"resources/engine/Shaders/Primitive.VS.hlsl", L"vs_6_0");
		ShaderModule primitivePS(ShaderStage::Pixel, L"resources/engine/Shaders/Primitive.PS.hlsl", L"ps_6_0");

		const ShaderReflection& PrimitiveVsReflection = primitiveVS.GetReflection();
		const ShaderReflection& PrimitivePsReflection = primitivePS.GetReflection();

		std::vector<ShaderReflection> primitiveRefls;
		primitiveRefls.push_back(PrimitiveVsReflection);
		primitiveRefls.push_back(PrimitivePsReflection);

		RootSignature primitiveRootSignature;
		std::wstring defaultPrimitivePSOName = L"Renderer : Primitive PSO";
		RootSignatureBuilder::BuildFromReflection(primitiveRefls, primitiveRootSignature, ConvertString(defaultPrimitivePSOName));

		D3D12_RASTERIZER_DESC primitiveRasterizerDesc{};
		primitiveRasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
		primitiveRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

		D3D12_BLEND_DESC blendPrimitiveDesc = {};
		blendPrimitiveDesc.IndependentBlendEnable = FALSE;
		blendPrimitiveDesc.RenderTarget[0].BlendEnable = TRUE;
		blendPrimitiveDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendPrimitiveDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		blendPrimitiveDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendPrimitiveDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		blendPrimitiveDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
		blendPrimitiveDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		blendPrimitiveDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		D3D12_DEPTH_STENCIL_DESC primitiveDepthStencilDesc{};
		primitiveDepthStencilDesc.DepthEnable = false;
		primitiveDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		primitiveDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

		std::vector<D3D12_INPUT_ELEMENT_DESC> primitiveInputLayout = InputLayoutBuilder::BuildFromReflection(PrimitiveVsReflection);


		GraphicsPSO defaultPrimitivePSO(defaultPrimitivePSOName);

		defaultPrimitivePSO.SetRootSignature(primitiveRootSignature);
		defaultPrimitivePSO.SetRasterizerState(primitiveRasterizerDesc);
		defaultPrimitivePSO.SetBlendState(blendPrimitiveDesc);
		defaultPrimitivePSO.SetDepthStencilState(depthStencilDesc);
		defaultPrimitivePSO.SetInputLayout(primitiveInputLayout);
		defaultPrimitivePSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE);
		defaultPrimitivePSO.SetRenderTargetFormats(1, rtvFormat, DXGI_FORMAT_D24_UNORM_S8_UINT);
		defaultPrimitivePSO.SetVertexShader(primitiveVS.GetBytecode());
		defaultPrimitivePSO.SetPixelShader(primitivePS.GetBytecode());
		defaultPrimitivePSO.SetSampleMask(D3D12_DEFAULT_SAMPLE_MASK);
		defaultPrimitivePSO.Finalize();
		ctx.RegisterGraphicsPSO(ConvertString(defaultPrimitivePSOName), defaultPrimitivePSO);
		ctx.RegisterRootSignature(ConvertString(defaultPrimitivePSOName), std::move(primitiveRootSignature));

	}
}
}