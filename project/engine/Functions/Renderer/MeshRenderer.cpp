#include "MeshRenderer.h"

#include "engine/Runtime/PipelineStateObject/GraphicsPSO.h"
#include "engine/Runtime/PipelineStateObject/RootSignature.h"
#include "engine/Functions/Shader/ShaderModule.h"
#include "engine/Runtime/GpuResource/GpuBuffer.h"
#include "engine/Assets/Material.h"
#include "engine/Math/Types/Transform.h"
#include "engine/Functions/Camera/Camera.h"
#include "engine/Assets/Texture/TextureManager.h"
#include "engine/Assets/Mesh.h"
#include "engine/Math/MathInclude.h"
#include "RenderSystem.h"
#include "RenderPass/SpritePass.h"
#include "engine/Assets/ModelLoader.h"
#include "engine/Functions/ECS/Registry.h"
#include "engine/Functions/Renderer/RenderPass/MeshPass.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif // USE_IMGUI


namespace NoEngine {

using namespace std;

namespace {
vector<GraphicsPSO> sGraphicsPSOs;
RootSignature sRootSig;

// 描画テスト用
unique_ptr<ByteAddressBuffer> vertexResource;
unique_ptr<ByteAddressBuffer> materialResource;
D3D12_VERTEX_BUFFER_VIEW vbv{};
Vertex triangle[] =
{
	{{-0.5f,-0.5f,0.0f,1.f},{0.f,1.f}},
	{{0.0f,0.5f,0.0f,1.f},{0.5f,0.f}},
	{{0.5f,-0.5f,0.0f,1.f},{1.f,1.f}},

	{{-0.5f,-0.5f,0.5f,1.f},{0.f,1.f}},
	{{0.0f,0.0f,0.0f,1.f},{0.5f,0.f}},
	{{0.5f,-0.5f,-0.5f,1.f},{1.f,1.f}}
};
Material material{};
Matrix4x4 worldData{};
Matrix4x4 CameraData{};
Transform transform;
float angle;
std::unique_ptr<Camera> camera;
std::unique_ptr <TextureRef> texRf;
std::unique_ptr <ECS::Registry> registry;
ECS::Entity en;
ECS::Entity en2;
Transform ct;
std::unique_ptr<Render::MeshPass> meshPass;
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

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = InputLayoutBuilder::BuildFromReflection(vsReflection);

	GraphicsPSO defaultPSO(L"Renderer : Default PSO");
	defaultPSO.SetRootSignature(sRootSig);
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

	gTextureHeap.Create(L"Scene Texture Descriptors", D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4096);

	// 三角形の描画テスト用初期化を行います。
	vertexResource = make_unique<ByteAddressBuffer>();
	
	material.color = { 1.f,1.f,1.f,1.f };
	worldData = Matrix4x4::IDENTITY;
	CameraData = Matrix4x4::IDENTITY;
	transform = Transform();
	angle = 0.f;
	
	texRf = std::make_unique<TextureRef>(TextureManager::LoadCovertTexture("resources/engine/Model/enemy.png"));
	meshPass = std::make_unique<Render::MeshPass>();
	Renderer::Initialize();
	registry = std::make_unique<ECS::Registry>();
	en = registry->GenerateEntity();
	en2 = registry->GenerateEntity();
	registry->AddComponent<Component::TransformComponent>(en);
	auto* model = registry->AddComponent<Component::MeshComponent>(en);
	model->mesh = ModelLoader::LoadModel("enemy", "resources/engine/Model/enemy.obj");

	auto m = registry->AddComponent<Component::MaterialComponent>(en);
	m->textureHandle = TextureManager::LoadCovertTexture("resources/engine/Model/enemy.png");
	m->pso = &sGraphicsPSOs.back();

	
	auto* model2 = registry->AddComponent<Component::MeshComponent>(en2);
	model2->mesh = ModelLoader::LoadModel("enemy", "resources/engine/Model/enemy.obj"); 
	
	auto* trans2 = registry->AddComponent<Component::TransformComponent>(en2);
	trans2->translate.x = 1.f;


	auto m2 = registry->AddComponent<Component::MaterialComponent>(en2);
	m2->textureHandle = TextureManager::LoadCovertTexture("resources/engine/Model/enemy.png");
	m2->pso = &sGraphicsPSOs.back();
}

void MeshRenderer::Shutdown() {
	registry.reset();
	meshPass.reset();
	texRf.reset();
	ModelLoader::DeleteAll();
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
		
		ct.rotation = { 0.f,0.f,0.f,1.f };
		ct.scale = { 1.f,1.f,1.f };
		ct.translate = { 0.f,0.f,-5.f };
		camera->SetTransform(ct);
	}
	camera->Update();
#ifdef USE_IMGUI
	ImGui::Begin("camera");
	ImGui::DragFloat3("pos", &ct.translate.x,0.1f);
	ImGui::End();
	camera->SetTransform(ct);
	auto* a = registry->GetComponent<Component::TransformComponent>(en);
	auto* b = registry->GetComponent<Component::MaterialComponent>(en);
	angle += 0.01f;
	a->rotation.FromAxisAngle(Vector3(0.f, 1.f, 0.f), angle);
	ImGui::Begin("model");
	ImGui::DragFloat3("translate", &a->translate.x, 0.05f);
	ImGui::DragFloat3("scale", &a->scale.x, 0.05f);
	ImGui::DragFloat4("rotate", &a->rotation.x, 0.04f);
	ImGui::DragFloat4("uv", &b->uv.x, 0.01f);
	ImGui::End();
#endif // USE_IMGUI
	meshPass->SetCamera(camera.get());

	context.SetRootSignature(sRootSig);
	context.SetPipelineState(sGraphicsPSOs.back());

	meshPass->Execute(context, *registry);

}
}