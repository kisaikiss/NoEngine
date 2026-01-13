#include "RenderSystem.h"
namespace NoEngine {
namespace Renderer {
GraphicsPSO gDefaultSpritePSO(L"Renderer : Default Sprite PSO");
RootSignature gDefaultSpriteRootSignature;
void Renderer::Initialize() {
	ShaderModule::Initialize();

	ShaderModule defaultSpriteVS(ShaderStage::Vertex, L"resources/engine/Shaders/DefaultSprite.VS.hlsl", L"vs_6_0");
	ShaderModule defaultPS(ShaderStage::Pixel, L"resources/engine/Shaders/DefaultSprite.PS.hlsl", L"ps_6_0");

	const ShaderReflection& spriteVsReflection = defaultSpriteVS.GetReflection();
	const ShaderReflection& psReflection = defaultPS.GetReflection();

	std::vector<ShaderReflection> refls;
	refls.push_back(spriteVsReflection);
	refls.push_back(psReflection);
	RootSignatureBuilder::BuildFromReflection(refls, gDefaultSpriteRootSignature, "defaultSpriteRootSignature");

	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	D3D12_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = false;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_NONE;

	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = InputLayoutBuilder::BuildFromReflection(spriteVsReflection);

	gDefaultSpritePSO.SetRootSignature(gDefaultSpriteRootSignature);
	gDefaultSpritePSO.SetRasterizerState(rasterizerDesc);
	gDefaultSpritePSO.SetBlendState(blendDesc);
	gDefaultSpritePSO.SetDepthStencilState(depthStencilDesc);
	gDefaultSpritePSO.SetInputLayout(inputLayout);
	gDefaultSpritePSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	DXGI_FORMAT rtvFormat[] = { DXGI_FORMAT_R8G8B8A8_UNORM_SRGB };
	gDefaultSpritePSO.SetRenderTargetFormats(1, rtvFormat, DXGI_FORMAT_UNKNOWN);
	gDefaultSpritePSO.SetVertexShader(defaultSpriteVS.GetBytecode());
	gDefaultSpritePSO.SetPixelShader(defaultPS.GetBytecode());
	gDefaultSpritePSO.SetSampleMask(D3D12_DEFAULT_SAMPLE_MASK);
	gDefaultSpritePSO.Finalize();
}


}
}