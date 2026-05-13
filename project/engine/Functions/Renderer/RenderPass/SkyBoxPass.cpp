#include "stdafx.h"
#include "SkyBoxPass.h"
#include "engine/Functions/Shader/ShaderModule.h"
#include "engine/Utilities/Conversion/ConvertString.h"

namespace NoEngine {
namespace Render {

namespace {

Math::Matrix4x4 sWorldMatrix;

}

SkyBoxPass::SkyBoxPass() {
	InitVertices();
	InitPSO();
	Transform transform;
	transform.scale = 1000.f;
	sWorldMatrix.MakeAffine(transform.scale, transform.rotation, transform.translate);
	texture_ = TextureManager::LoadTextureFile("resources/engine/Texture/rostock_laage_airport_4k.dds", Graphics::kMagenta2D, true);
}

void SkyBoxPass::Execute(GraphicsContext& gfx, ECS::Registry& registry) {
	using namespace Component;
	auto cameraView = registry.View<CameraComponent, ActiveCameraTag>();
	for (auto entity : cameraView) {
		camera_ = registry.GetComponent<CameraComponent>(entity);
	}

	auto& rootIndex = RootSignatureBuilder::GetRootIndexMap("SkyBox PSO");
	gfx.SetPipelineState(pso_);
	gfx.SetRootSignature(rootSig_);

	gfx.SetDynamicConstantBufferView(rootIndex["gWorldMatrix"], sizeof(Math::Matrix4x4), &sWorldMatrix);
	gfx.SetDynamicConstantBufferView(rootIndex["gCameraMatrix"], sizeof(Component::CameraForGPU), &camera_->forGPU);
	gfx.SetDynamicConstantBufferView(rootIndex["gMaterial"], sizeof(Math::Color), &Math::Color::WHITE);
	gfx.SetVertexBuffer(0, vertex_.VertexBufferView());
	gfx.SetIndexBuffer(index_.IndexBufferView());
	gfx.SetDynamicDescriptor(rootIndex["gTexture"], 0, texture_.GetSRV());

	gfx.DrawIndexed(36);
}

void SkyBoxPass::InitVertices() {
	Math::Vector3 vertexData[24];
	uint32_t indexData[36];
	// 右面
	vertexData[0] = { 1.f, 1.f, 1.f };
	vertexData[1] = { 1.f, 1.f, -1.f };
	vertexData[2] = { 1.f, -1.f, 1.f };
	vertexData[3] = { 1.f, -1.f, -1.f };

	indexData[0] = 0;
	indexData[1] = 1;
	indexData[2] = 2;

	indexData[3] = 2;
	indexData[4] = 1;
	indexData[5] = 3;

	// 左面
	vertexData[4] = { -1.f, 1.f, -1.f };
	vertexData[5] = { -1.f, 1.f, 1.f };
	vertexData[6] = { -1.f, -1.f, -1.f };
	vertexData[7] = { -1.f, -1.f, 1.f };

	indexData[6] = 4;
	indexData[7] = 5;
	indexData[8] = 6;

	indexData[9] = 6;
	indexData[10] = 5;
	indexData[11] = 7;

	// 前面
	vertexData[8] = { -1.f, 1.f, 1.f };
	vertexData[9] = { 1.f, 1.f, 1.f };
	vertexData[10] = { -1.f, -1.f, 1.f };
	vertexData[11] = { 1.f, -1.f, 1.f };

	indexData[12] = 8;
	indexData[13] = 9;
	indexData[14] = 10;

	indexData[15] = 10;
	indexData[16] = 9;
	indexData[17] = 11;

	// 後面
	vertexData[12] = { 1.f,  1.f, -1.f };
	vertexData[13] = { -1.f,  1.f, -1.f };
	vertexData[14] = { 1.f, -1.f, -1.f };
	vertexData[15] = { -1.f, -1.f, -1.f };

	indexData[18] = 12;
	indexData[19] = 13;
	indexData[20] = 14;

	indexData[21] = 14;
	indexData[22] = 13;
	indexData[23] = 15;

	// 上面
	vertexData[16] = { -1.f, 1.f, -1.f };
	vertexData[17] = { 1.f, 1.f, -1.f };
	vertexData[18] = { -1.f, 1.f,  1.f };
	vertexData[19] = { 1.f, 1.f,  1.f };

	indexData[24] = 16;
	indexData[25] = 17;
	indexData[26] = 18;

	indexData[27] = 18;
	indexData[28] = 17;
	indexData[29] = 19;

	// 下面
	vertexData[20] = { -1.f, -1.f,  1.f };
	vertexData[21] = { 1.f, -1.f,  1.f };
	vertexData[22] = { -1.f, -1.f, -1.f };
	vertexData[23] = { 1.f, -1.f, -1.f };

	indexData[30] = 20;
	indexData[31] = 21;
	indexData[32] = 22;

	indexData[33] = 22;
	indexData[34] = 21;
	indexData[35] = 23;

	UploadBuffer vertexUpload;
	vertexUpload.Create(L"VertexUpload", sizeof(vertexData));
	memcpy(vertexUpload.Map(), vertexData, sizeof(vertexData));
	vertexUpload.Unmap();
	vertex_.Create(L"SkyBoxVertex", 24, sizeof(Math::Vector3), vertexUpload);

	UploadBuffer indexUpload;
	indexUpload.Create(L"IndexUpload", sizeof(indexData));
	memcpy(indexUpload.Map(), indexData, sizeof(indexData));
	indexUpload.Unmap();
	index_.Create(L"SkyBoxIndex", sizeof(indexData), sizeof(uint32_t), indexUpload);
}

void SkyBoxPass::InitPSO() {
	ShaderModule defaultVS(ShaderStage::Vertex, L"resources/engine/Shaders/SkyBox.VS.hlsl", L"vs_6_0");
	ShaderModule defaultPS(ShaderStage::Pixel, L"resources/engine/Shaders/SkyBox.PS.hlsl", L"ps_6_0");

	const ShaderReflection& vsReflection = defaultVS.GetReflection();
	const ShaderReflection& psReflection = defaultPS.GetReflection();

	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	D3D12_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	DXGI_FORMAT rtvFormat[] = { DXGI_FORMAT_R8G8B8A8_UNORM_SRGB };

	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = InputLayoutBuilder::BuildFromReflection(vsReflection);

	std::vector<ShaderReflection> refls;
	refls.push_back(vsReflection);
	refls.push_back(psReflection);

	rootSig_;
	std::wstring defaultPSOName = L"SkyBox PSO";
	RootSignatureBuilder::BuildFromReflection(refls, rootSig_, ConvertString(defaultPSOName));

	pso_ = GraphicsPSO(defaultPSOName);
	pso_.SetRootSignature(rootSig_);
	pso_.SetRasterizerState(rasterizerDesc);
	pso_.SetBlendState(blendDesc);
	pso_.SetDepthStencilState(depthStencilDesc);
	pso_.SetInputLayout(inputLayout);
	pso_.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

	pso_.SetRenderTargetFormats(1, rtvFormat, DXGI_FORMAT_D24_UNORM_S8_UINT);
	pso_.SetVertexShader(defaultVS.GetBytecode());
	pso_.SetPixelShader(defaultPS.GetBytecode());
	pso_.SetSampleMask(D3D12_DEFAULT_SAMPLE_MASK);
	pso_.Finalize();
}

}
}