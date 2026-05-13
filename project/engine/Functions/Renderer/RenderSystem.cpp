#include "RenderSystem.h"
#include "engine/Assets/ModelLoader.h"
#include "engine/Utilities/Conversion/ConvertString.h"
#include "engine/Functions/Shader/ShaderModule.h"
#include "engine/Runtime/GraphicsCore.h"

namespace NoEngine {
namespace Render {
DescriptorHeap gTextureHeap;

namespace {
std::vector<GraphicsPSO> sGraphicsPSOs;
std::unordered_map<std::wstring, uint32_t> sGraphicsPSOIndexMap;

std::vector<std::unique_ptr<RootSignature>> sRootSignatures;
std::unordered_map<std::wstring, uint32_t> sRootSignatureIndexMap;

Microsoft::WRL::ComPtr<ID3D12StateObject> sRtShadowStateObject;
D3D12_DISPATCH_RAYS_DESC sShadowDispatchRaysDesc;

Microsoft::WRL::ComPtr<ID3D12Resource> sShadowShaderTable;

Microsoft::WRL::ComPtr<ID3D12StateObject> sRtTestStateObject;
D3D12_DISPATCH_RAYS_DESC sTestDispatchRaysDesc;

Microsoft::WRL::ComPtr<ID3D12Resource> sTestShaderTable;
}

void Initialize() {
	gTextureHeap.Create(L"Scene Texture Descriptors", D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4096);
	// ToDo : 現在はシェーダーコンパイル、PSO生成をここで行っていますが、アプリケーション側で動的に行えるようにするべきです。
	ShaderModule::Initialize();

	// よく使う設定
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

		std::unique_ptr<RootSignature> defaultRootSignature = std::make_unique<RootSignature>();
		std::wstring defaultPSOName = L"Renderer : Default PSO";
		RootSignatureBuilder::BuildFromReflection(refls, *defaultRootSignature, ConvertString(defaultPSOName));

		GraphicsPSO defaultPSO(defaultPSOName);
		defaultPSO.SetRootSignature(*defaultRootSignature);
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
		sGraphicsPSOs.push_back(defaultPSO);
		sGraphicsPSOIndexMap[defaultPSOName] = static_cast<uint32_t>(sGraphicsPSOs.size()) - 1;
		sRootSignatures.push_back(std::move(defaultRootSignature));
		sRootSignatureIndexMap[defaultPSOName] = static_cast<uint32_t>(sRootSignatures.size()) - 1;
	}
	

	// スキニング
	{
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
		skinnedInputLayout[4].Format = DXGI_FORMAT_R32G32B32A32_UINT;


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
	}

	// PreRenderPass
	{
		ShaderModule pixelShader(ShaderStage::Pixel, L"resources/engine/Shaders/PreRender.PS.hlsl", L"ps_6_0");

		const ShaderReflection& prePSReflection = pixelShader.GetReflection();
		std::vector<ShaderReflection> refls;
		refls.push_back(vsReflection);
		refls.push_back(prePSReflection);

		std::unique_ptr<RootSignature> defaultRootSignature = std::make_unique<RootSignature>();
		std::wstring defaultPSOName = L"Renderer : PreRender PSO";
		RootSignatureBuilder::BuildFromReflection(refls, *defaultRootSignature, ConvertString(defaultPSOName));

		GraphicsPSO defaultPSO(defaultPSOName);
		defaultPSO.SetRootSignature(*defaultRootSignature);
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
		sGraphicsPSOs.push_back(defaultPSO);
		sGraphicsPSOIndexMap[defaultPSOName] = static_cast<uint32_t>(sGraphicsPSOs.size()) - 1;
		sRootSignatures.push_back(std::move(defaultRootSignature));
		sRootSignatureIndexMap[defaultPSOName] = static_cast<uint32_t>(sRootSignatures.size()) - 1;
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

		std::unique_ptr<RootSignature> defaultSkinnedRootSignature = std::make_unique<RootSignature>();
		std::wstring defaultSkinnedPSOName = L"Renderer : PreRenderSkinned PSO";
		RootSignatureBuilder::BuildFromReflection(skinnedRefls, *defaultSkinnedRootSignature, ConvertString(defaultSkinnedPSOName));

		std::vector<D3D12_INPUT_ELEMENT_DESC> skinnedInputLayout = InputLayoutBuilder::BuildFromReflection(vsSkinnedReflection);
		// ToDo : inputLayoutのReflectionがUINT型のインプットが対応できていないので、このように後から入れる形になってしまっています。UINT型に対応すべきです。
		skinnedInputLayout[4].Format = DXGI_FORMAT_R32G32B32A32_UINT;


		GraphicsPSO defaultSkinnedPSO(defaultSkinnedPSOName);
		defaultSkinnedPSO.SetRootSignature(*defaultSkinnedRootSignature);
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
		sGraphicsPSOs.push_back(defaultSkinnedPSO);
		sGraphicsPSOIndexMap[defaultSkinnedPSOName] = static_cast<uint32_t>(sGraphicsPSOs.size()) - 1;
		sRootSignatures.push_back(std::move(defaultSkinnedRootSignature));
		sRootSignatureIndexMap[defaultSkinnedPSOName] = static_cast<uint32_t>(sRootSignatures.size()) - 1;
	}

	// トゥーンレンダリング
	ShaderModule ToonPS(ShaderStage::Pixel, L"resources/engine/Shaders/Toon.PS.hlsl", L"ps_6_0");
	const ShaderReflection& ToonReflection = ToonPS.GetReflection();
	{
		std::vector<ShaderReflection> reflectionToon;
		reflectionToon.push_back(vsReflection);
		reflectionToon.push_back(ToonReflection);
		std::unique_ptr<RootSignature> toonRootSignature = std::make_unique<RootSignature>();
		std::wstring toonPSOName = L"Renderer : Toon PSO";
		RootSignatureBuilder::BuildFromReflection(reflectionToon, *toonRootSignature, ConvertString(toonPSOName));

		GraphicsPSO toonPSO(toonPSOName);

		toonPSO.SetRootSignature(*toonRootSignature);
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
		sGraphicsPSOs.push_back(toonPSO);
		sGraphicsPSOIndexMap[toonPSOName] = static_cast<uint32_t>(sGraphicsPSOs.size()) - 1;
		sRootSignatures.push_back(std::move(toonRootSignature));
		sRootSignatureIndexMap[toonPSOName] = static_cast<uint32_t>(sRootSignatures.size()) - 1;
	}

	// スキニングトゥーンレンダリング
	{
		ShaderModule defaultSkinnedVS(ShaderStage::Vertex, L"resources/engine/Shaders/DefaultSkinned.VS.hlsl", L"vs_6_0");

		const ShaderReflection& vsSkinnedReflection = defaultSkinnedVS.GetReflection();
		std::vector<ShaderReflection> skinnedRefls;
		skinnedRefls.push_back(vsSkinnedReflection);
		skinnedRefls.push_back(ToonReflection);

		std::unique_ptr<RootSignature> defaultSkinnedRootSignature = std::make_unique<RootSignature>();
		std::wstring defaultSkinnedPSOName = L"Renderer : ToonSkinned PSO";
		RootSignatureBuilder::BuildFromReflection(skinnedRefls, *defaultSkinnedRootSignature, ConvertString(defaultSkinnedPSOName));

		std::vector<D3D12_INPUT_ELEMENT_DESC> skinnedInputLayout = InputLayoutBuilder::BuildFromReflection(vsSkinnedReflection);
		// ToDo : inputLayoutのReflectionがUINT型のインプットが対応できていないので、このように後から入れる形になってしまっています。UINT型に対応すべきです。
		skinnedInputLayout[4].Format = DXGI_FORMAT_R32G32B32A32_UINT;


		GraphicsPSO defaultSkinnedPSO(defaultSkinnedPSOName);
		defaultSkinnedPSO.SetRootSignature(*defaultSkinnedRootSignature);
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
		sGraphicsPSOs.push_back(defaultSkinnedPSO);
		sGraphicsPSOIndexMap[defaultSkinnedPSOName] = static_cast<uint32_t>(sGraphicsPSOs.size()) - 1;
		sRootSignatures.push_back(std::move(defaultSkinnedRootSignature));
		sRootSignatureIndexMap[defaultSkinnedPSOName] = static_cast<uint32_t>(sRootSignatures.size()) - 1;
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
		std::unique_ptr<RootSignature> outlineRootSignature = std::make_unique<RootSignature>();
		std::wstring outlinePSOName = L"Renderer : outline PSO";
		RootSignatureBuilder::BuildFromReflection(reflectionOutline, *outlineRootSignature, ConvertString(outlinePSOName));

		D3D12_RASTERIZER_DESC rasterizerOutlineDesc{};
		rasterizerOutlineDesc.CullMode = D3D12_CULL_MODE_FRONT;
		rasterizerOutlineDesc.FillMode = D3D12_FILL_MODE_SOLID;

		std::vector<D3D12_INPUT_ELEMENT_DESC> outlineInputLayout = InputLayoutBuilder::BuildFromReflection(outlineVsReflection);

		GraphicsPSO outlinePSO(outlinePSOName);

		outlinePSO.SetRootSignature(*outlineRootSignature);
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
		sGraphicsPSOs.push_back(outlinePSO);
		sGraphicsPSOIndexMap[outlinePSOName] = static_cast<uint32_t>(sGraphicsPSOs.size()) - 1;
		sRootSignatures.push_back(std::move(outlineRootSignature));
		sRootSignatureIndexMap[outlinePSOName] = static_cast<uint32_t>(sRootSignatures.size()) - 1;
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
		std::unique_ptr<RootSignature> outlineRootSignature = std::make_unique<RootSignature>();
		std::wstring outlinePSOName = L"Renderer : skinnedOutline PSO";
		RootSignatureBuilder::BuildFromReflection(reflectionOutline, *outlineRootSignature, ConvertString(outlinePSOName));

		D3D12_RASTERIZER_DESC rasterizerOutlineDesc{};
		rasterizerOutlineDesc.CullMode = D3D12_CULL_MODE_FRONT;
		rasterizerOutlineDesc.FillMode = D3D12_FILL_MODE_SOLID;

		std::vector<D3D12_INPUT_ELEMENT_DESC> outlineInputLayout = InputLayoutBuilder::BuildFromReflection(outlineVsReflection);
		outlineInputLayout[4].Format = DXGI_FORMAT_R32G32B32A32_UINT;

		GraphicsPSO outlinePSO(outlinePSOName);

		outlinePSO.SetRootSignature(*outlineRootSignature);
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
		sGraphicsPSOs.push_back(outlinePSO);
		sGraphicsPSOIndexMap[outlinePSOName] = static_cast<uint32_t>(sGraphicsPSOs.size()) - 1;
		sRootSignatures.push_back(std::move(outlineRootSignature));
		sRootSignatureIndexMap[outlinePSOName] = static_cast<uint32_t>(sRootSignatures.size()) - 1;
	}

	// Particle
	{
		ShaderModule particleVS(ShaderStage::Vertex, L"resources/engine/Shaders/Particle.VS.hlsl", L"vs_6_0");
		ShaderModule particlePS(ShaderStage::Pixel, L"resources/engine/Shaders/Particle.PS.hlsl", L"ps_6_0");

		const ShaderReflection& particleVsReflection = particleVS.GetReflection();
		const ShaderReflection& particlePsReflection = particlePS.GetReflection();

		std::vector<ShaderReflection> reflection;
		reflection.push_back(particleVsReflection);
		reflection.push_back(particlePsReflection);
		std::unique_ptr<RootSignature> particleRootSignature = std::make_unique<RootSignature>();
		std::wstring particlePSOName = L"Renderer : particle PSO";
		RootSignatureBuilder::BuildFromReflection(reflection, *particleRootSignature, ConvertString(particlePSOName));

		D3D12_BLEND_DESC blendParticleDesc{};
		blendParticleDesc.IndependentBlendEnable = FALSE;
		blendParticleDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		blendParticleDesc.RenderTarget[0].BlendEnable = TRUE;
		blendParticleDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendParticleDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		blendParticleDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendParticleDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		blendParticleDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
		blendParticleDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;

		D3D12_DEPTH_STENCIL_DESC depthStencilParticleDesc{};
		depthStencilParticleDesc.DepthEnable = true;
		depthStencilParticleDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
		depthStencilParticleDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

		std::vector<D3D12_INPUT_ELEMENT_DESC> particleInputLayout = InputLayoutBuilder::BuildFromReflection(particleVsReflection);
		// ShadarReflectionがInstanceIDを認識してしまうので削除する
		particleInputLayout.erase(particleInputLayout.end() - 1);

		GraphicsPSO particlePSO(particlePSOName);

		particlePSO.SetRootSignature(*particleRootSignature);
		particlePSO.SetRasterizerState(rasterizerDesc);
		particlePSO.SetBlendState(blendParticleDesc);
		particlePSO.SetDepthStencilState(depthStencilParticleDesc);
		particlePSO.SetInputLayout(particleInputLayout);
		particlePSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
		particlePSO.SetRenderTargetFormats(1, rtvFormat, DXGI_FORMAT_D24_UNORM_S8_UINT);
		particlePSO.SetVertexShader(particleVS.GetBytecode());
		particlePSO.SetPixelShader(particlePS.GetBytecode());
		particlePSO.SetSampleMask(D3D12_DEFAULT_SAMPLE_MASK);
		particlePSO.Finalize();
		sGraphicsPSOs.push_back(particlePSO);
		sGraphicsPSOIndexMap[particlePSOName] = static_cast<uint32_t>(sGraphicsPSOs.size()) - 1;
		sRootSignatures.push_back(std::move(particleRootSignature));
		sRootSignatureIndexMap[particlePSOName] = static_cast<uint32_t>(sRootSignatures.size()) - 1;
	}

	// sprite
	{

		ShaderModule defaultSpriteVS(ShaderStage::Vertex, L"resources/engine/Shaders/DefaultSprite.VS.hlsl", L"vs_6_0");
		ShaderModule defaultSpritePS(ShaderStage::Pixel, L"resources/engine/Shaders/DefaultSprite.PS.hlsl", L"ps_6_0");

		const ShaderReflection& spriteVsReflection = defaultSpriteVS.GetReflection();
		const ShaderReflection& spritePsReflection = defaultSpritePS.GetReflection();

		std::vector<ShaderReflection> reflectionSprite;
		reflectionSprite.push_back(spriteVsReflection);
		reflectionSprite.push_back(spritePsReflection);
		std::unique_ptr<RootSignature> defaultSpriteRootSignature = std::make_unique<RootSignature>();
		std::wstring defaultSpritePSOName = L"Renderer : Default Sprite PSO";
		RootSignatureBuilder::BuildFromReflection(reflectionSprite, *defaultSpriteRootSignature, ConvertString(defaultSpritePSOName));

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

		defaultSpritePSO.SetRootSignature(*defaultSpriteRootSignature);
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
		sGraphicsPSOs.push_back(defaultSpritePSO);
		sGraphicsPSOIndexMap[defaultSpritePSOName] = static_cast<uint32_t>(sGraphicsPSOs.size()) - 1;
		sRootSignatures.push_back(std::move(defaultSpriteRootSignature));
		sRootSignatureIndexMap[defaultSpritePSOName] = static_cast<uint32_t>(sRootSignatures.size()) - 1;
	}

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
		defaultPrimitivePSO.SetDepthStencilState(depthStencilDesc);
		defaultPrimitivePSO.SetInputLayout(primitiveInputLayout);
		defaultPrimitivePSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE);
		defaultPrimitivePSO.SetRenderTargetFormats(1, rtvFormat, DXGI_FORMAT_D24_UNORM_S8_UINT);
		defaultPrimitivePSO.SetVertexShader(primitiveVS.GetBytecode());
		defaultPrimitivePSO.SetPixelShader(primitivePS.GetBytecode());
		defaultPrimitivePSO.SetSampleMask(D3D12_DEFAULT_SAMPLE_MASK);
		defaultPrimitivePSO.Finalize();
		sGraphicsPSOs.push_back(defaultPrimitivePSO);
		sGraphicsPSOIndexMap[defaultPrimitivePSOName] = static_cast<uint32_t>(sGraphicsPSOs.size()) - 1;
		sRootSignatures.push_back(std::move(primitiveRootSignature));
		sRootSignatureIndexMap[defaultPrimitivePSOName] = static_cast<uint32_t>(sRootSignatures.size()) - 1;

	}

	// RayTest
	{
		ShaderModule raytracingLib(
			ShaderStage::RaytraceLib,
			L"resources/engine/Shaders/Raytracing/RaytracingTestLib.hlsl",
			L"lib_6_3",
			true);

		InitRaytracingGlobalRootSignature();

		std::vector<D3D12_STATE_SUBOBJECT> subObjects;
		subObjects.reserve(5);

		// 1. DXIL ライブラリ
		D3D12_EXPORT_DESC exports[3] = {};

		// RayGen
		exports[0].Name = L"RayGen";
		exports[0].ExportToRename = nullptr;
		exports[0].Flags = D3D12_EXPORT_FLAG_NONE;

		// Miss
		exports[1].Name = L"MissShader";
		exports[1].ExportToRename = nullptr;
		exports[1].Flags = D3D12_EXPORT_FLAG_NONE;

		// ClosestHit
		exports[2].Name = L"ClosestHitShader";
		exports[2].ExportToRename = nullptr;
		exports[2].Flags = D3D12_EXPORT_FLAG_NONE;


		D3D12_DXIL_LIBRARY_DESC dxilLib = {};
		dxilLib.DXILLibrary = raytracingLib.GetBytecode();
		dxilLib.NumExports = _countof(exports);
		dxilLib.pExports = exports;

		D3D12_STATE_SUBOBJECT libSubobject = {};
		libSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
		libSubobject.pDesc = &dxilLib;
		subObjects.push_back(libSubobject);

		// 2. HitGroup
		static const wchar_t* kHitGroupExport = L"TestHitGroup";
		static const wchar_t* kClosestHitExport = L"ClosestHitShader";

		D3D12_HIT_GROUP_DESC hitGroup = {};
		hitGroup.HitGroupExport = kHitGroupExport;
		hitGroup.ClosestHitShaderImport = kClosestHitExport;
		hitGroup.AnyHitShaderImport = nullptr;
		hitGroup.IntersectionShaderImport = nullptr;
		hitGroup.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;

		D3D12_STATE_SUBOBJECT hitGroupSubobject = {};
		hitGroupSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
		hitGroupSubobject.pDesc = &hitGroup;
		subObjects.push_back(hitGroupSubobject);

		// 3. ShaderConfig（Association なし＝全シェーダーに適用）
		D3D12_RAYTRACING_SHADER_CONFIG shaderConfig = {};
		shaderConfig.MaxPayloadSizeInBytes = 16;                 // float 4つ
		shaderConfig.MaxAttributeSizeInBytes = sizeof(float) * 2; // barycentrics

		D3D12_STATE_SUBOBJECT shaderConfigSubobject = {};
		shaderConfigSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
		shaderConfigSubobject.pDesc = &shaderConfig;
		subObjects.push_back(shaderConfigSubobject);

		// 4. Global RootSignature
		// 構造体を作成し、そこにRootSignatureのポインタをセットする

		std::unique_ptr<RootSignature> rtGlobalRSptr = std::make_unique<RootSignature>();
		auto& rtGlobalRS = *rtGlobalRSptr.get();

		rtGlobalRS.Reset(3, 0);

		// TLAS
		rtGlobalRS[0].InitAsBufferSRV(0);

		// UAV
		rtGlobalRS[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 1);

		// CBV b0
		rtGlobalRS[2].InitAsConstantBuffer(0);


		rtGlobalRS.Finalize(L"RT Test Global RootSignature");
		sRootSignatures.push_back(std::move(rtGlobalRSptr));
		sRootSignatureIndexMap[L"RT Test Global RootSignature"] = static_cast<uint32_t>(sRootSignatures.size()) - 1;

		D3D12_GLOBAL_ROOT_SIGNATURE globalRSDesc = {};
		globalRSDesc.pGlobalRootSignature = sRootSignatures[sRootSignatureIndexMap[L"RT Test Global RootSignature"]]->GetSignature();

		D3D12_STATE_SUBOBJECT globalRSSubobject = {};
		globalRSSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
		// 構造体のアドレスを渡す
		globalRSSubobject.pDesc = &globalRSDesc;
		subObjects.push_back(globalRSSubobject);

		// 5. PipelineConfig
		D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig = {};
		pipelineConfig.MaxTraceRecursionDepth = 1;

		D3D12_STATE_SUBOBJECT pipelineConfigSubobject = {};
		pipelineConfigSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
		pipelineConfigSubobject.pDesc = &pipelineConfig;
		subObjects.push_back(pipelineConfigSubobject);

		// 6. StateObject 生成
		D3D12_STATE_OBJECT_DESC desc = {};
		desc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
		desc.NumSubobjects = static_cast<UINT>(subObjects.size());
		desc.pSubobjects = subObjects.data();

		HRESULT hr = GraphicsCore::sGraphicsDevice->GetDevice()->CreateStateObject(&desc, IID_PPV_ARGS(&sRtTestStateObject));

		if (FAILED(hr)) {
			assert(false);
		}
		

		using namespace Microsoft::WRL;
		// 1. シェーダー識別子を取る
		ComPtr<ID3D12StateObjectProperties> props;
		sRtTestStateObject.As(&props);

		void* raygenId = props->GetShaderIdentifier(L"RayGen");
		void* missId = props->GetShaderIdentifier(L"MissShader");
		void* hitId = props->GetShaderIdentifier(L"TestHitGroup");

		// 2.ShaderTableバッファ作成
		const UINT shaderIdSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES; // 32
		const UINT recordStride = (shaderIdSize + (D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT - 1))
			& ~(D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT - 1); // 32

		const UINT tableAlignment = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT; // 64

		// 各テーブルが64バイトアライメントの境界から始まるようにセクションサイズを計算
		const UINT rayGenSectionSize = (recordStride + tableAlignment - 1) & ~(tableAlignment - 1);   // 64
		const UINT missSectionSize = (recordStride + tableAlignment - 1) & ~(tableAlignment - 1);   // 64
		const UINT hitGroupSectionSize = (recordStride + tableAlignment - 1) & ~(tableAlignment - 1); // 64

		// オフセットの計算
		const UINT rayGenOffset = 0;
		const UINT missOffset = rayGenOffset + rayGenSectionSize;     // 64
		const UINT hitGroupOffset = missOffset + missSectionSize;         // 128

		const UINT totalTableSizeUnaligned = hitGroupOffset + hitGroupSectionSize; // 192

		// バッファ先頭のアライメント調整用の余白(+64)を含めたサイズ
		const UINT tableSize = totalTableSizeUnaligned + tableAlignment;

		// Create upload buffer
		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(tableSize);
		GraphicsCore::sGraphicsDevice->GetDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&sTestShaderTable));

		// Map and write identifiers at offsets relative to an aligned base
		uint8_t* mapped = nullptr;
		sTestShaderTable->Map(0, nullptr, reinterpret_cast<void**>(&mapped));
		UINT64 gpuBase = sTestShaderTable->GetGPUVirtualAddress();

		// compute aligned GPU base (>= gpuBase) aligned to 64
		UINT64 alignedGpuBase = (gpuBase + (tableAlignment - 1)) & ~(UINT64)(tableAlignment - 1);

		// compute CPU offset corresponding to alignedGpuBase
		SIZE_T cpuOffset = static_cast<SIZE_T>(alignedGpuBase - gpuBase);

		// 計算したアライメント済みのオフセットを使って書き込む
		memcpy(mapped + cpuOffset + rayGenOffset, raygenId, shaderIdSize);
		memcpy(mapped + cpuOffset + missOffset, missId, shaderIdSize);
		memcpy(mapped + cpuOffset + hitGroupOffset, hitId, shaderIdSize);

		sTestShaderTable->Unmap(0, nullptr);

		// Fill dispatch desc using alignedGpuBase
		auto& dispatchRayDesc = sTestDispatchRaysDesc;

		// 各StartAddressに計算したオフセットを足す
		dispatchRayDesc.RayGenerationShaderRecord.StartAddress = alignedGpuBase + rayGenOffset;
		dispatchRayDesc.RayGenerationShaderRecord.SizeInBytes = recordStride;

		dispatchRayDesc.MissShaderTable.StartAddress = alignedGpuBase + missOffset;
		dispatchRayDesc.MissShaderTable.SizeInBytes = recordStride;
		dispatchRayDesc.MissShaderTable.StrideInBytes = recordStride;

		dispatchRayDesc.HitGroupTable.StartAddress = alignedGpuBase + hitGroupOffset;
		dispatchRayDesc.HitGroupTable.SizeInBytes = recordStride;
		dispatchRayDesc.HitGroupTable.StrideInBytes = recordStride;

		dispatchRayDesc.Width = 1280;
		dispatchRayDesc.Height = 720;
		dispatchRayDesc.Depth = 1;
	}

	// RayShadow
	{
		
		ShaderModule raytracingLib(
			ShaderStage::RaytraceLib, 
			L"resources/engine/Shaders/Raytracing/RayTracingShadowLib.hlsl",
			L"lib_6_3",
			true);
	
		InitRaytracingGlobalRootSignature();

		std::vector<D3D12_STATE_SUBOBJECT> subObjects;
		subObjects.reserve(5);

		// 1. DXIL ライブラリ
		D3D12_EXPORT_DESC exports[3] = {};

		// RayGen
		exports[0].Name = L"RayGen_Shadow";
		exports[0].ExportToRename = nullptr;
		exports[0].Flags = D3D12_EXPORT_FLAG_NONE;

		// Miss
		exports[1].Name = L"Miss_Shadow";
		exports[1].ExportToRename = nullptr;
		exports[1].Flags = D3D12_EXPORT_FLAG_NONE;

		// ClosestHit
		exports[2].Name = L"ClosestHit_Shadow";
		exports[2].ExportToRename = nullptr;
		exports[2].Flags = D3D12_EXPORT_FLAG_NONE;


		D3D12_DXIL_LIBRARY_DESC dxilLib = {};
		dxilLib.DXILLibrary = raytracingLib.GetBytecode();
		dxilLib.NumExports = _countof(exports);
		dxilLib.pExports = exports;

		D3D12_STATE_SUBOBJECT libSubobject = {};
		libSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
		libSubobject.pDesc = &dxilLib;
		subObjects.push_back(libSubobject);

		// 2. HitGroup
		static const wchar_t* kHitGroupExport = L"ShadowHitGroup";
		static const wchar_t* kClosestHitExport = L"ClosestHit_Shadow";

		D3D12_HIT_GROUP_DESC hitGroup = {};
		hitGroup.HitGroupExport = kHitGroupExport;
		hitGroup.ClosestHitShaderImport = kClosestHitExport;
		hitGroup.AnyHitShaderImport = nullptr;
		hitGroup.IntersectionShaderImport = nullptr;
		hitGroup.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;

		D3D12_STATE_SUBOBJECT hitGroupSubobject = {};
		hitGroupSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
		hitGroupSubobject.pDesc = &hitGroup;
		subObjects.push_back(hitGroupSubobject);

		// 3. ShaderConfig（Association なし＝全シェーダーに適用）
		D3D12_RAYTRACING_SHADER_CONFIG shaderConfig = {};
		shaderConfig.MaxPayloadSizeInBytes = 4;                 // bool 1つ
		shaderConfig.MaxAttributeSizeInBytes = sizeof(float) * 2; // barycentrics

		D3D12_STATE_SUBOBJECT shaderConfigSubobject = {};
		shaderConfigSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
		shaderConfigSubobject.pDesc = &shaderConfig;
		subObjects.push_back(shaderConfigSubobject);

		// 4. Global RootSignature
		// 構造体を作成し、そこにRootSignatureのポインタをセットする
		D3D12_GLOBAL_ROOT_SIGNATURE globalRSDesc = {};
		globalRSDesc.pGlobalRootSignature = sRootSignatures[sRootSignatureIndexMap[L"RT Global RootSignature"]]->GetSignature();

		D3D12_STATE_SUBOBJECT globalRSSubobject = {};
		globalRSSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
		// 構造体のアドレスを渡す
		globalRSSubobject.pDesc = &globalRSDesc;
		subObjects.push_back(globalRSSubobject);

		// 5. PipelineConfig
		D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig = {};
		pipelineConfig.MaxTraceRecursionDepth = 1;

		D3D12_STATE_SUBOBJECT pipelineConfigSubobject = {};
		pipelineConfigSubobject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
		pipelineConfigSubobject.pDesc = &pipelineConfig;
		subObjects.push_back(pipelineConfigSubobject);

		// 6. StateObject 生成
		D3D12_STATE_OBJECT_DESC desc = {};
		desc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
		desc.NumSubobjects = static_cast<UINT>(subObjects.size());
		desc.pSubobjects = subObjects.data();

		HRESULT hr = GraphicsCore::sGraphicsDevice->GetDevice()->CreateStateObject(&desc, IID_PPV_ARGS(&sRtShadowStateObject));

		if (FAILED(hr)) {
			assert(false);
		}
		CreateShadowShaderTable();
	}
	
}

void Shutdown() {
	sShadowShaderTable.Reset();
	sRtShadowStateObject.Reset();

	sTestShaderTable.Reset();
	sRtTestStateObject.Reset();

	gTextureHeap.Destroy();
	Asset::ModelLoader::DeleteAll();
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

Microsoft::WRL::ComPtr<ID3D12StateObject>& GetShadowRtStateObject() {
	return sRtShadowStateObject;
}

D3D12_DISPATCH_RAYS_DESC& GetShadowDispatchRaysDesc() {
	return sShadowDispatchRaysDesc;
}

Microsoft::WRL::ComPtr<ID3D12StateObject>& GetRtStateObject() {
	return sRtTestStateObject;
}

D3D12_DISPATCH_RAYS_DESC& GetDispatchRaysDesc() {
	return sTestDispatchRaysDesc;
}

void CreateShadowShaderTable() {
	using namespace Microsoft::WRL;
	// 1. シェーダー識別子を取る
	ComPtr<ID3D12StateObjectProperties> props;
	sRtShadowStateObject.As(&props);

	void* raygenId = props->GetShaderIdentifier(L"RayGen_Shadow");
	void* missId = props->GetShaderIdentifier(L"Miss_Shadow");
	void* hitId = props->GetShaderIdentifier(L"ShadowHitGroup");

	// 2.ShaderTableバッファ作成
	const UINT shaderIdSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES; // 32
	const UINT recordStride = (shaderIdSize + (D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT - 1))
		& ~(D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT - 1); // 32

	const UINT tableAlignment = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT; // 64

	// 各テーブルが64バイトアライメントの境界から始まるようにセクションサイズを計算
	const UINT rayGenSectionSize = (recordStride + tableAlignment - 1) & ~(tableAlignment - 1);   // 64
	const UINT missSectionSize = (recordStride + tableAlignment - 1) & ~(tableAlignment - 1);   // 64
	const UINT hitGroupSectionSize = (recordStride + tableAlignment - 1) & ~(tableAlignment - 1); // 64

	// オフセットの計算
	const UINT rayGenOffset = 0;
	const UINT missOffset = rayGenOffset + rayGenSectionSize;     // 64
	const UINT hitGroupOffset = missOffset + missSectionSize;         // 128

	const UINT totalTableSizeUnaligned = hitGroupOffset + hitGroupSectionSize; // 192

	// バッファ先頭のアライメント調整用の余白(+64)を含めたサイズ
	const UINT tableSize = totalTableSizeUnaligned + tableAlignment;

	// Create upload buffer
	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(tableSize);
	GraphicsCore::sGraphicsDevice->GetDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&sShadowShaderTable));

	// Map and write identifiers at offsets relative to an aligned base
	uint8_t* mapped = nullptr;
	sShadowShaderTable->Map(0, nullptr, reinterpret_cast<void**>(&mapped));
	UINT64 gpuBase = sShadowShaderTable->GetGPUVirtualAddress();

	// compute aligned GPU base (>= gpuBase) aligned to 64
	UINT64 alignedGpuBase = (gpuBase + (tableAlignment - 1)) & ~(UINT64)(tableAlignment - 1);

	// compute CPU offset corresponding to alignedGpuBase
	SIZE_T cpuOffset = static_cast<SIZE_T>(alignedGpuBase - gpuBase);

	// 計算したアライメント済みのオフセットを使って書き込む
	memcpy(mapped + cpuOffset + rayGenOffset, raygenId, shaderIdSize);
	memcpy(mapped + cpuOffset + missOffset, missId, shaderIdSize);
	memcpy(mapped + cpuOffset + hitGroupOffset, hitId, shaderIdSize);

	sShadowShaderTable->Unmap(0, nullptr);

	// Fill dispatch desc using alignedGpuBase
	auto& desc = sShadowDispatchRaysDesc;

	// 各StartAddressに計算したオフセットを足す
	desc.RayGenerationShaderRecord.StartAddress = alignedGpuBase + rayGenOffset;
	desc.RayGenerationShaderRecord.SizeInBytes = recordStride;

	desc.MissShaderTable.StartAddress = alignedGpuBase + missOffset;
	desc.MissShaderTable.SizeInBytes = recordStride;
	desc.MissShaderTable.StrideInBytes = recordStride;

	desc.HitGroupTable.StartAddress = alignedGpuBase + hitGroupOffset;
	desc.HitGroupTable.SizeInBytes = recordStride;
	desc.HitGroupTable.StrideInBytes = recordStride;

	desc.Width = 1280;
	desc.Height = 720;
	desc.Depth = 1;
}

void InitRaytracingGlobalRootSignature() {
	std::unique_ptr<RootSignature> rtGlobalRSptr = std::make_unique<RootSignature>();
	auto& rtGlobalRS = *rtGlobalRSptr.get();

	rtGlobalRS.Reset(7, 0);

	// TLAS
	rtGlobalRS[0].InitAsBufferSRV(0);

	// SRV テーブル（worldPos）
	rtGlobalRS[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1);

	// Lights
	rtGlobalRS[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 1);

	// UAV
	rtGlobalRS[3].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 1);

	// CBV b0
	rtGlobalRS[4].InitAsConstantBuffer(0);

	// CBV b1
	rtGlobalRS[5].InitAsConstantBuffer(1);

	// Normal
	rtGlobalRS[6].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 3, 1);


	rtGlobalRS.Finalize(L"RT Global RootSignature");
	sRootSignatures.push_back(std::move(rtGlobalRSptr));
	sRootSignatureIndexMap[L"RT Global RootSignature"] = static_cast<uint32_t>(sRootSignatures.size()) - 1;
}

void InitRaytracingLocalRootSignature() {
	std::unique_ptr<RootSignature> rtLocalRSptr = std::make_unique<RootSignature>();
	auto& rtLocalRS = *rtLocalRSptr.get();

	rtLocalRS.Reset(1, 0);

	// materialIndex, instanceID の 2 つを渡す
	rtLocalRS[0].InitAsConstants(
		0,      // b0
		2,      // 2 DWORD
		D3D12_SHADER_VISIBILITY_ALL,
		1       // space1 (local root)
	);

	rtLocalRS.Finalize(
		L"RT Local RootSignature",
		D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE
	);

	sRootSignatures.push_back(std::move(rtLocalRSptr));
	sRootSignatureIndexMap[L"RT Local RootSignature"] = static_cast<uint32_t>(sRootSignatures.size()) - 1;
}
}
}