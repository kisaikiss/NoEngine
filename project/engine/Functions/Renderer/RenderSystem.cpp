#include "RenderSystem.h"
#include "engine/Assets/ModelLoader.h"
#include "engine/Utilities/Conversion/ConvertString.h"
#include "engine/Functions/Shader/ShaderModule.h"

namespace NoEngine {
namespace Render {
DescriptorHeap gTextureHeap;

namespace {
std::vector<GraphicsPSO> sGraphicsPSOs;
std::unordered_map<std::wstring, uint32_t> sGraphicsPSOIndexMap;

std::vector<std::unique_ptr<RootSignature>> sRootSignatures;
std::unordered_map<std::wstring, uint32_t> sRootSignatureIndexMap;
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
	std::wstring defaultPSOName = L"Renderer : Default PSO";
	RootSignatureBuilder::BuildFromReflection(refls, *defaultRootSignature, ConvertString(defaultPSOName));

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
	sGraphicsPSOIndexMap[defaultPSOName] = static_cast<uint32_t>(sGraphicsPSOs.size()) - 1;
	sRootSignatures.push_back(std::move(defaultRootSignature));
	sRootSignatureIndexMap[defaultPSOName] = static_cast<uint32_t>(sRootSignatures.size()) - 1;

	ShaderModule defaultSkinnedVS(ShaderStage::Vertex, L"resources/engine/Shaders/DefaultSkinned.VS.hlsl", L"vs_6_0");

	const ShaderReflection& vsSkinnedReflection = defaultSkinnedVS.GetReflection();
	std::vector<ShaderReflection> skinnedRefls;
	skinnedRefls.push_back(vsSkinnedReflection);
	skinnedRefls.push_back(psReflection);

	std::unique_ptr<RootSignature> defaultSkinnedRootSignature = std::make_unique<RootSignature>();
	std::wstring defaultSkinnedPSOName = L"Renderer : DefaultSkinned PSO";
	RootSignatureBuilder::BuildFromReflection(skinnedRefls, *defaultSkinnedRootSignature, ConvertString(defaultSkinnedPSOName));

	std::vector<D3D12_INPUT_ELEMENT_DESC> skinnedInputLayout = InputLayoutBuilder::BuildFromReflection(vsSkinnedReflection);
	// ToDo : inputLayoutのReflectionがUINT型のインプットが対応できていないので、このように後から入れる形になってしまっています。UINT型に対応すべきです。
	skinnedInputLayout[3].Format = DXGI_FORMAT_R32G32B32A32_UINT;

	
	GraphicsPSO defaultSkinnedPSO(defaultSkinnedPSOName);
	defaultSkinnedPSO.SetRootSignature(*defaultSkinnedRootSignature);
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
	sGraphicsPSOs.push_back(defaultSkinnedPSO);
	sGraphicsPSOIndexMap[defaultSkinnedPSOName] = static_cast<uint32_t>(sGraphicsPSOs.size()) - 1;
	sRootSignatures.push_back(std::move(defaultSkinnedRootSignature));
	sRootSignatureIndexMap[defaultSkinnedPSOName] = static_cast<uint32_t>(sRootSignatures.size()) - 1;


	ShaderModule defaultSpriteVS(ShaderStage::Vertex, L"resources/engine/Shaders/DefaultSprite.VS.hlsl", L"vs_6_0");
	
	const ShaderReflection& spriteVsReflection = defaultSpriteVS.GetReflection();

	std::vector<ShaderReflection> reflectionSprite;
	reflectionSprite.push_back(spriteVsReflection);
	reflectionSprite.push_back(psReflection);
	std::unique_ptr<RootSignature> defaultSpriteRootSignature = std::make_unique<RootSignature>();
	std::wstring defaultSpritePSOName = L"Renderer : Default Sprite PSO";
	RootSignatureBuilder::BuildFromReflection(reflectionSprite, *defaultSpriteRootSignature, ConvertString(defaultSpritePSOName));
	
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
	sGraphicsPSOIndexMap[defaultSpritePSOName] = static_cast<uint32_t>(sGraphicsPSOs.size()) - 1;
	sRootSignatures.push_back(std::move(defaultSpriteRootSignature));
	sRootSignatureIndexMap[defaultSpritePSOName] = static_cast<uint32_t>(sRootSignatures.size()) - 1;

	//primitive
	{
		ShaderModule primitiveVS(ShaderStage::Vertex, L"resources/engine/Shaders/Primitive.VS.hlsl", L"vs_6_0");
		ShaderModule primitivePS(ShaderStage::Pixel, L"resources/engine/Shaders/Primitive.PS.hlsl", L"ps_6_0");

		const ShaderReflection& PrimitiveVsReflection = primitiveVS.GetReflection();
		const ShaderReflection& PrimitivePsReflection = primitivePS.GetReflection();

		std::vector<ShaderReflection> primitiveRefls;
		primitiveRefls.push_back(PrimitiveVsReflection);
		primitiveRefls.push_back(PrimitivePsReflection);

		std::unique_ptr<RootSignature> primitiveRootSignature = std::make_unique<RootSignature>();
		std::wstring defaultPrimitivePSOName = L"Renderer : Primitive PSO";
		RootSignatureBuilder::BuildFromReflection(primitiveRefls, *primitiveRootSignature, ConvertString(defaultPrimitivePSOName));

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

		defaultPrimitivePSO.SetRootSignature(*primitiveRootSignature);
		defaultPrimitivePSO.SetRasterizerState(primitiveRasterizerDesc);
		defaultPrimitivePSO.SetBlendState(blendPrimitiveDesc);
		defaultPrimitivePSO.SetDepthStencilState(primitiveDepthStencilDesc);
		defaultPrimitivePSO.SetInputLayout(primitiveInputLayout);
		defaultPrimitivePSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE);
		defaultPrimitivePSO.SetRenderTargetFormats(1, rtvFormat, DXGI_FORMAT_UNKNOWN);
		defaultPrimitivePSO.SetVertexShader(primitiveVS.GetBytecode());
		defaultPrimitivePSO.SetPixelShader(primitivePS.GetBytecode());
		defaultPrimitivePSO.SetSampleMask(D3D12_DEFAULT_SAMPLE_MASK);
		defaultPrimitivePSO.Finalize();
		sGraphicsPSOs.push_back(defaultPrimitivePSO);
		sGraphicsPSOIndexMap[defaultPrimitivePSOName] = static_cast<uint32_t>(sGraphicsPSOs.size()) - 1;
		sRootSignatures.push_back(std::move(primitiveRootSignature));
		sRootSignatureIndexMap[defaultPrimitivePSOName] = static_cast<uint32_t>(sRootSignatures.size()) - 1;

	}
	
}

void Shutdown() {
	gTextureHeap.Destroy();
	ModelLoader::DeleteAll();
	PSO::DestroyAll();
	RootSignature::DestroyAll();
	ShaderModule::Shutdown();
}

GraphicsPSO& GetPSO(uint32_t psoId) {
	return sGraphicsPSOs[psoId];
}

RootSignature& GetRootSignature(uint32_t rootSigId) {
	return *sRootSignatures[rootSigId];
}

uint32_t GetPSOID(std::wstring psoName) {
	return sGraphicsPSOIndexMap[psoName];
}

uint32_t GetRootSignatureID(std::wstring rootSigName) {
	return sRootSignatureIndexMap[rootSigName];
}

}
}