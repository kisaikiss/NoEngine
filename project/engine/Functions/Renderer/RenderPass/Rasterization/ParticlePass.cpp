#include "stdafx.h"
#include "ParticlePass.h"
#include "engine/Runtime/GpuResource/UploadBuffer.h"
#include "engine/Functions/Shader/ShaderReflection.h"

namespace NoEngine {
namespace Render {

ParticlePass::ParticlePass() {
	vertex_ = ByteAddressBuffer();
	ParticleVertex vertices[] = {
		{{-0.5f,-0.5f,0.f,1.f},{0,1}},
		{{0.5f,-0.5f,0.f,1.f},{1,1}},
		{{-0.5f,0.5f,0.f,1.f},{0,0}},
		{{0.5f,0.5f,0.f,1.f},{1,0}},
	};

	UploadBuffer vertexUpload;
	vertexUpload.Create(L"VertexUpload", sizeof(vertices));
	memcpy(vertexUpload.Map(), vertices, sizeof(vertices));
	vertexUpload.Unmap();
	vertex_.Create(L"partilceVertex", 4, sizeof(ParticleVertex), vertexUpload);

	uint32_t indices[] = {
		0,1,2,1,3,2
	};
	UploadBuffer indexUpload;
	indexUpload.Create(L"IndexUpload", sizeof(indices));
	memcpy(indexUpload.Map(), indices, sizeof(indices));
	indexUpload.Unmap();
	index_.Create(L"particleIndex", sizeof(indices), sizeof(uint32_t), indexUpload);



	maxParticles_ = 50000;
	Initialize(maxParticles_);
}

void ParticlePass::Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) {
	static_cast<void>(resourceRegistry);
	auto* renderCtx = GetRenderContext();
	camera_ = GetTargetCamera();
	if (!camera_) return;
	auto& rootIndex = RootSignatureBuilder::GetRootIndexMap("Renderer : particle PSO");

	// GPUへ行列を転送
	UploadMatrices(gfx, registry);

	if (items_.empty()) return;

	gfx.SetPipelineState(renderCtx->GetGraphicsPSO("Renderer : particle PSO"));
	gfx.SetRootSignature(renderCtx->GetRootSignature("Renderer : particle PSO"));
	gfx.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	gfx.SetVertexBuffer(0, vertex_.VertexBufferView());
	gfx.SetIndexBuffer(index_.IndexBufferView());

	// カメラ
	gfx.SetDynamicConstantBufferView(rootIndex["gCameraMatrix"], sizeof(Component::CameraForGPU), &camera_->forGPU);

	// ワールド行列
	gfx.SetDynamicDescriptor(
		rootIndex["gWorldMatrices"],
		0,
		worldMatrixBuffer_.GetSRV()
	);


	uint32_t count = 0;
	for (size_t i = 0; i < baseIndices_.size() - 1; i++) {
		size_t baseIndex = baseIndices_[i];
		size_t nextIndex = baseIndices_[i + 1];
		size_t instanceCount = nextIndex - baseIndex; // インスタンス数を計算

		// baseIndex
		_declspec(align(16)) struct {
			uint32_t index;
			uint32_t pad[3];
		} indexConstant;
		indexConstant.index = (uint32_t)baseIndex;
		gfx.SetDynamicConstantBufferView(rootIndex["gBaseIndex"], sizeof(indexConstant), &indexConstant);

		gfx.SetDynamicDescriptor(rootIndex["gTexture"], 0, textures_[count]->GetSRV());

		gfx.DrawIndexedInstanced(6, static_cast<UINT>(instanceCount), 0, 0, static_cast<UINT>(baseIndex));

		count++;
	}
	
	particleCount_ = 0;
	particleForGpu_.clear();
}

void ParticlePass::UploadMatrices(GraphicsContext& gfx, ECS::Registry& registry) {
	auto view = registry.View<Component::ParticleComponent, Component::TransformComponent>();

	Math::Matrix4x4 billBoardMatrix{};
	{
		Math::Matrix4x4 backToFrontMatrix;
		backToFrontMatrix.MakeRotate(Math::Vector3::UP * PI);
		auto* transform = registry.GetComponent<Component::TransformComponent>(GetTargetCamera()->entity);
		billBoardMatrix = backToFrontMatrix * transform->MakeAffineMatrix4x4();
		billBoardMatrix.m[3][0] = 0.0f;
		billBoardMatrix.m[3][1] = 0.0f;
		billBoardMatrix.m[3][2] = 0.0f;
	}
	

	
	// collect
	items_.clear();
	for (auto entity : view) {
		auto* particle = registry.GetComponent<Component::ParticleComponent>(entity);
		auto* transform = registry.GetComponent<Component::TransformComponent>(entity);

		items_.push_back({ transform,particle });
	}

	if (items_.empty()) return;

	// sort
	std::sort(items_.begin(), items_.end(), [](const DrawItem& a, const DrawItem& b) {
		return a.particle->texture < b.particle->texture;
		});

	// build
	size_t baseIndex = 0;
	baseIndices_.clear();
	TextureRef currentTexture;
	textures_.clear();
	for (auto& item : items_) {
		if (currentTexture != item.particle->texture) {
			currentTexture = item.particle->texture;
			baseIndices_.push_back(baseIndex);
			textures_.push_back(item.particle->texture);
		}

		Math::Matrix4x4 scaleMatrix;
		scaleMatrix.MakeScale(item.transform->scale);
		Math::Matrix4x4 translateMatrix;
		translateMatrix.MakeTranslate(item.transform->translate);
		Math::Matrix4x4 worldMatrix = scaleMatrix * billBoardMatrix * translateMatrix;
		particleForGpu_.push_back({ worldMatrix, item.particle->color });

		baseIndex++;
	}

	baseIndices_.push_back(baseIndex);



	size_t count = std::min(maxParticles_, baseIndex);
	if (count == 0) return;

	size_t size = count * sizeof(ParticleForGPU);
	memcpy(worldMatrixUpload_.Map(), particleForGpu_.data(), size);
	worldMatrixUpload_.Unmap();
	gfx.CopyBufferRegion(
		worldMatrixBuffer_,
		0,
		worldMatrixUpload_,
		0,
		size
	);

}

void ParticlePass::Initialize(size_t maxParticles) {
	size_t bufferSize = maxParticles * sizeof(ParticleForGPU);

	worldMatrixBuffer_.Create(
		L"ParticleWorldMatrices",
		static_cast<uint32_t>(maxParticles),
		sizeof(ParticleForGPU)
	);

	worldMatrixUpload_.Create(
		L"ParticleWorldMatricesUpload",
		bufferSize
	);

}

}
}