#include "RenderSystem.h"
#include "engine/Assets/ModelLoader.h"

namespace NoEngine {
namespace Render {
DescriptorHeap gTextureHeap;

namespace {
std::vector<GraphicsPSO> sGraphicsPSOs;
std::unordered_map<std::wstring, size_t> sGraphicsPSOIndexMap;

std::vector<std::unique_ptr<RootSignature>> sRootSignatures;
std::unordered_map<std::string, size_t> sRootSignatureIndexMap;
}

void Initialize() {
	gTextureHeap.Create(L"Scene Texture Descriptors", D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4096);
	// ToDo : 現在はシェーダーコンパイル、PSO生成をここで行っていますが、アプリケーション側で動的に行えるようにするべきです。
	ShaderModule::Initialize();
	ShaderModule defaultVS(ShaderStage::Vertex, L"resources/engine/Shaders/Default.VS.hlsl", L"vs_6_0");
	ShaderModule defaultPS(ShaderStage::Pixel, L"resources/engine/Shaders/Default.PS.hlsl", L"ps_6_0");

	const ShaderReflection& vsReflection = defaultVS.GetReflection();
	const ShaderReflection& psReflection = defaultPS.GetReflection();
	std::vector<ShaderReflection> refls;
	refls.push_back(vsReflection);
	refls.push_back(psReflection);

	std::unique_ptr<RootSignature> defaultRootSignature = std::make_unique<RootSignature>();
	std::string defaultRootSignatureName = "defaultRootSignature";
	RootSignatureBuilder::BuildFromReflection(refls, *defaultRootSignature, defaultRootSignatureName);

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

	std::wstring defaultPSOName = L"Renderer : Default PSO";
	GraphicsPSO defaultPSO(defaultPSOName);
	defaultPSO.SetRootSignature(*defaultRootSignature);
	defaultPSO.SetRasterizerState(rasterizerDesc);
	defaultPSO.SetBlendState(blendDesc);
	defaultPSO.SetDepthStencilState(depthStencilDesc);
	defaultPSO.SetInputLayout(inputLayout);
	defaultPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	DXGI_FORMAT rtvFormat[] = { DXGI_FORMAT_R8G8B8A8_UNORM_SRGB };
	defaultPSO.SetRenderTargetFormats(1, rtvFormat, DXGI_FORMAT_D24_UNORM_S8_UINT);
	defaultPSO.SetVertexShader(defaultVS.GetBytecode());
	defaultPSO.SetPixelShader(defaultPS.GetBytecode());
	defaultPSO.SetSampleMask(D3D12_DEFAULT_SAMPLE_MASK);
	defaultPSO.Finalize();
	sGraphicsPSOs.push_back(defaultPSO);
	sGraphicsPSOIndexMap[defaultPSOName] = sGraphicsPSOs.size() - 1;
	sRootSignatures.push_back(std::move(defaultRootSignature));
	sRootSignatureIndexMap[defaultRootSignatureName] = sRootSignatures.size() - 1;




	ShaderModule defaultSpriteVS(ShaderStage::Vertex, L"resources/engine/Shaders/DefaultSprite.VS.hlsl", L"vs_6_0");
	
	const ShaderReflection& spriteVsReflection = defaultSpriteVS.GetReflection();

	std::vector<ShaderReflection> reflectionSprite;
	reflectionSprite.push_back(spriteVsReflection);
	reflectionSprite.push_back(psReflection);
	std::unique_ptr<RootSignature> defaultSpriteRootSignature = std::make_unique<RootSignature>();
	std::string defaultSpriteRootSignatureName = "defaultSpriteRootSignature";
	RootSignatureBuilder::BuildFromReflection(reflectionSprite, *defaultSpriteRootSignature, defaultSpriteRootSignatureName);
	
	D3D12_RASTERIZER_DESC rasterizerSpriteDesc{};
	rasterizerSpriteDesc.CullMode = D3D12_CULL_MODE_NONE;
	rasterizerSpriteDesc.FillMode = D3D12_FILL_MODE_SOLID;

	D3D12_BLEND_DESC blendSpriteDesc{};
	blendSpriteDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	D3D12_DEPTH_STENCIL_DESC depthStencilSpriteDesc{};
	depthStencilSpriteDesc.DepthEnable = false;
	depthStencilSpriteDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilSpriteDesc.DepthFunc = D3D12_COMPARISON_FUNC_NONE;

	std::vector<D3D12_INPUT_ELEMENT_DESC> spriteInputLayout = InputLayoutBuilder::BuildFromReflection(spriteVsReflection);

	std::wstring defaultSpritePSOName = L"Renderer : Default Sprite PSO";
	GraphicsPSO defaultSpritePSO(defaultSpritePSOName);

	defaultSpritePSO.SetRootSignature(*defaultSpriteRootSignature);
	defaultSpritePSO.SetRasterizerState(rasterizerSpriteDesc);
	defaultSpritePSO.SetBlendState(blendSpriteDesc);
	defaultSpritePSO.SetDepthStencilState(depthStencilSpriteDesc);
	defaultSpritePSO.SetInputLayout(spriteInputLayout);
	defaultSpritePSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	defaultSpritePSO.SetRenderTargetFormats(1, rtvFormat, DXGI_FORMAT_UNKNOWN);
	defaultSpritePSO.SetVertexShader(defaultSpriteVS.GetBytecode());
	defaultSpritePSO.SetPixelShader(defaultPS.GetBytecode());
	defaultSpritePSO.SetSampleMask(D3D12_DEFAULT_SAMPLE_MASK);
	defaultSpritePSO.Finalize();
	sGraphicsPSOs.push_back(defaultSpritePSO);
	sGraphicsPSOIndexMap[defaultSpritePSOName] = sGraphicsPSOs.size() - 1;
	sRootSignatures.push_back(std::move(defaultSpriteRootSignature));
	sRootSignatureIndexMap[defaultSpriteRootSignatureName] = sRootSignatures.size() - 1;

}

void Shutdown() {
	gTextureHeap.Destroy();
	ModelLoader::DeleteAll();
	PSO::DestroyAll();
	RootSignature::DestroyAll();
	ShaderModule::Shutdown();
}

GraphicsPSO& GetPSO(std::wstring psoName) {
	return sGraphicsPSOs[sGraphicsPSOIndexMap[psoName]];
}

RootSignature& GetRootSignature(std::string rootSigName) {
	return *sRootSignatures[sRootSignatureIndexMap[rootSigName]];
}

}
}