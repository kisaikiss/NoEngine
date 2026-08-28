#include "stdafx.h"
#include "SpriteInitializer.h"
#include "engine/Functions/Shader/ShaderModule.h"
#include "engine/Utilities/Conversion/ConvertString.h"
namespace NoEngine {
void SpriteInitializer::Initialize(RenderContext& ctx) {
	CreatePSO(ctx);
}

void SpriteInitializer::CreatePSO(RenderContext& ctx) {
	{
		DXGI_FORMAT rtvFormat[] = { DXGI_FORMAT_R8G8B8A8_UNORM_SRGB };
		ShaderModule defaultSpriteVS(ShaderStage::Vertex, L"resources/engine/Shaders/DefaultSprite.VS.hlsl", L"vs_6_0");
		ShaderModule defaultSpritePS(ShaderStage::Pixel, L"resources/engine/Shaders/DefaultSprite.PS.hlsl", L"ps_6_0");

		const ShaderReflection& spriteVsReflection = defaultSpriteVS.GetReflection();
		const ShaderReflection& spritePsReflection = defaultSpritePS.GetReflection();

		std::vector<ShaderReflection> reflectionSprite;
		reflectionSprite.push_back(spriteVsReflection);
		reflectionSprite.push_back(spritePsReflection);
		RootSignature defaultSpriteRootSignature;
		std::wstring defaultSpritePSOName = L"Renderer : Default Sprite PSO";
		RootSignatureBuilder::BuildFromReflection(reflectionSprite, defaultSpriteRootSignature, ConvertString(defaultSpritePSOName));

		D3D12_RASTERIZER_DESC rasterizerSpriteDesc{};
		rasterizerSpriteDesc.CullMode = D3D12_CULL_MODE_NONE;
		rasterizerSpriteDesc.FillMode = D3D12_FILL_MODE_SOLID;

		D3D12_BLEND_DESC blendSpriteDesc{};
		blendSpriteDesc.IndependentBlendEnable = FALSE;
		blendSpriteDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		blendSpriteDesc.RenderTarget[0].BlendEnable = TRUE;
		blendSpriteDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendSpriteDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		blendSpriteDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendSpriteDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		blendSpriteDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
		blendSpriteDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;

		D3D12_DEPTH_STENCIL_DESC depthStencilSpriteDesc{};
		depthStencilSpriteDesc.DepthEnable = false;
		depthStencilSpriteDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
		depthStencilSpriteDesc.DepthFunc = D3D12_COMPARISON_FUNC_NONE;

		std::vector<D3D12_INPUT_ELEMENT_DESC> spriteInputLayout = InputLayoutBuilder::BuildFromReflection(spriteVsReflection);


		GraphicsPSO defaultSpritePSO(defaultSpritePSOName);

		defaultSpritePSO.SetRootSignature(defaultSpriteRootSignature);
		defaultSpritePSO.SetRasterizerState(rasterizerSpriteDesc);
		defaultSpritePSO.SetBlendState(blendSpriteDesc);
		defaultSpritePSO.SetDepthStencilState(depthStencilSpriteDesc);
		defaultSpritePSO.SetInputLayout(spriteInputLayout);
		defaultSpritePSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
		defaultSpritePSO.SetRenderTargetFormats(1, rtvFormat, DXGI_FORMAT_UNKNOWN);
		defaultSpritePSO.SetVertexShader(defaultSpriteVS.GetBytecode());
		defaultSpritePSO.SetPixelShader(defaultSpritePS.GetBytecode());
		defaultSpritePSO.SetSampleMask(D3D12_DEFAULT_SAMPLE_MASK);
		defaultSpritePSO.Finalize();
		ctx.RegisterGraphicsPSO(ConvertString(defaultSpritePSOName), defaultSpritePSO);
		ctx.RegisterRootSignature(ConvertString(defaultSpritePSOName), std::move(defaultSpriteRootSignature));
	}

	{
		ShaderModule spriteIdVS(ShaderStage::Vertex, L"resources/engine/Shaders/DefaultSprite.VS.hlsl", L"vs_6_0");
		ShaderModule spriteIdPS(ShaderStage::Pixel, L"resources/engine/Shaders/SpriteObjectID.PS.hlsl", L"ps_6_0");

		const ShaderReflection& spriteIdVsReflection = spriteIdVS.GetReflection();
		const ShaderReflection& spriteIdPsReflection = spriteIdPS.GetReflection();

		std::vector<ShaderReflection> reflectionSpriteId;
		reflectionSpriteId.push_back(spriteIdVsReflection);
		reflectionSpriteId.push_back(spriteIdPsReflection);
		RootSignature spriteIdRootSignature;
		std::wstring spriteIdPSOName = L"Renderer : Sprite ObjectID PSO";
		RootSignatureBuilder::BuildFromReflection(reflectionSpriteId, spriteIdRootSignature, ConvertString(spriteIdPSOName));

		D3D12_RASTERIZER_DESC rasterizerSpriteIdDesc{};
		rasterizerSpriteIdDesc.CullMode = D3D12_CULL_MODE_NONE;
		rasterizerSpriteIdDesc.FillMode = D3D12_FILL_MODE_SOLID;

		D3D12_BLEND_DESC blendSpriteIdDesc{};
		blendSpriteIdDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL; // ブレンドせずIDを上書き

		D3D12_DEPTH_STENCIL_DESC depthStencilSpriteIdDesc{};
		depthStencilSpriteIdDesc.DepthEnable = false;
		depthStencilSpriteIdDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
		depthStencilSpriteIdDesc.DepthFunc = D3D12_COMPARISON_FUNC_NONE;

		std::vector<D3D12_INPUT_ELEMENT_DESC> spriteIdInputLayout = InputLayoutBuilder::BuildFromReflection(spriteIdVsReflection);

		DXGI_FORMAT idRtvFormat[] = { DXGI_FORMAT_R8G8B8A8_UNORM };

		GraphicsPSO spriteIdPSO(spriteIdPSOName);
		spriteIdPSO.SetRootSignature(spriteIdRootSignature);
		spriteIdPSO.SetRasterizerState(rasterizerSpriteIdDesc);
		spriteIdPSO.SetBlendState(blendSpriteIdDesc);
		spriteIdPSO.SetDepthStencilState(depthStencilSpriteIdDesc);
		spriteIdPSO.SetInputLayout(spriteIdInputLayout);
		spriteIdPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
		spriteIdPSO.SetRenderTargetFormats(1, idRtvFormat, DXGI_FORMAT_UNKNOWN);
		spriteIdPSO.SetVertexShader(spriteIdVS.GetBytecode());
		spriteIdPSO.SetPixelShader(spriteIdPS.GetBytecode());
		spriteIdPSO.SetSampleMask(D3D12_DEFAULT_SAMPLE_MASK);
		spriteIdPSO.Finalize();
		ctx.RegisterGraphicsPSO(ConvertString(spriteIdPSOName), spriteIdPSO);
		ctx.RegisterRootSignature(ConvertString(spriteIdPSOName), std::move(spriteIdRootSignature));
	}
}
}