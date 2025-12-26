#include "MeshRenderer.h"

#include "engine/Runtime/PipelineStateObject/GraphicsPSO.h"
#include "engine/Runtime/PipelineStateObject/RootSignature.h"
#include "engine/Runtime/Shader/ShaderModule.h"
#include "engine/Runtime/GpuResource/GpuBuffer.h"
#include "engine/Math/Types/Vector4.h"
#include "engine/Assets/Material.h"
#include "engine/Math/Types/Matrix4x4.h"
#include "engine/Math/Types/Transform.h"
#include "engine/Functions/Camera/Camera.h"

namespace NoEngine {

using namespace std;

namespace {
vector<GraphicsPSO> sGraphicsPSOs;
RootSignature sRootSig;

// 描画テスト用
unique_ptr<ByteAddressBuffer> vertexResource;
unique_ptr<ByteAddressBuffer> materialResource;
D3D12_VERTEX_BUFFER_VIEW vbv{};
Vector4 triangle[] =
{
	{-0.5f,-0.5f,0.0f,1.f},
	{0.0f,0.5f,0.0f,1.f},
	{0.5f,-0.5f,0.0f,1.f}
};
Material material{};
Matrix4x4 wvpData{};
Transform transform;
float angle;
std::unique_ptr<Camera> camera;
}

DescriptorHeap MeshRenderer::gTextureHeap;

void MeshRenderer::Initialize() {
	// ToDo : 現在はシェーダーコンパイル、PSO生成をここで行っていますが、アプリケーション側で動的に行えるようにするべきです。
	ShaderModule::Initialize();
	ShaderModule defaultVS(ShaderStage::Vertex, L"resources/engine/Shaders/Default.VS.hlsl", L"vs_6_0");
	ShaderModule defaultPS(ShaderStage::Pixel, L"resources/engine/Shaders/Default.PS.hlsl", L"ps_6_0");

	const ShaderReflection& vsReflection = defaultVS.GetReflection();
	const ShaderReflection& psReflection = defaultPS.GetReflection();
	std::vector<ShaderReflection> refls;
	refls.push_back(vsReflection);
	refls.push_back(psReflection);
	RootSignatureBuilder::BuildFromReflection(refls, sRootSig, "defaultRootSig");

	D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	D3D12_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = InputLayoutBuilder::BuildFromReflection(vsReflection);

	GraphicsPSO defaultPSO(L"Renderer : Default PSO");
	defaultPSO.SetRootSignature(sRootSig);
	defaultPSO.SetRasterizerState(rasterizerDesc);
	defaultPSO.SetBlendState(blendDesc);
	defaultPSO.SetInputLayout(inputLayout);
	defaultPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
	DXGI_FORMAT rtvFormat[] = { DXGI_FORMAT_R8G8B8A8_UNORM_SRGB };
	defaultPSO.SetRenderTargetFormats(1, rtvFormat , DXGI_FORMAT_UNKNOWN);
	defaultPSO.SetVertexShader(defaultVS.GetBytecode());
	defaultPSO.SetPixelShader(defaultPS.GetBytecode());
	defaultPSO.SetSampleMask(D3D12_DEFAULT_SAMPLE_MASK);
	defaultPSO.Finalize();
	sGraphicsPSOs.push_back(defaultPSO);

	gTextureHeap.Create(L"Scene Texture Descriptors", D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4096);

	// 三角形の描画テスト用初期化を行います。
	vertexResource = make_unique<ByteAddressBuffer>();
	vertexResource->Create(L"vertex", sizeof(triangle), sizeof(Vector4), triangle);
	vbv = vertexResource->VertexBufferView(0,sizeof(triangle),sizeof(Vector4));
	material.color = { 1.f,1.f,0.f,1.f };
	wvpData = Matrix4x4::IDENTITY;
	transform = Transform();
	angle = 0.f;
	
}

void MeshRenderer::Shutdown() {
	gTextureHeap.Destroy();
	vertexResource->Destroy();
	vertexResource.reset();
	PSO::DestroyAll();
	RootSignature::DestroyAll();
	ShaderModule::Shutdown();
}

void MeshRenderer::Render(GraphicsContext& context) {
	if (!camera) {
		camera = std::make_unique<Camera>();
		Transform ct;
		ct.rotation = { 0.f,0.f,0.f,1.f };
		ct.scale = { 1.f,1.f,1.f };
		ct.translate = { 0.f,0.f,-5.f };
		camera->SetTransform(ct);
	}

	context.SetRootSignature(sRootSig);
	context.SetPipelineState(sGraphicsPSOs.back());

	context.SetVertexBuffer(0, vbv);
	context.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	std::unordered_map<std::string, uint32_t>& rootIndex = RootSignatureBuilder::GetRootIndexMap("defaultRootSig");
	context.SetDynamicConstantBufferView(rootIndex["gMaterial"], sizeof(Material), &material);
	angle += 0.05f;
	transform.rotation.FromAxisAngle(Vector3(0.f, 1.f, 0.f), angle);
	wvpData = transform.MakeAffineMatrix4x4();
	camera->Update();
	wvpData = wvpData * camera->GetViewProjMatrix();
	context.SetDynamicConstantBufferView(rootIndex["gTransformationMatrix"], sizeof(Matrix4x4), &wvpData);
	context.Draw(3);

}
}