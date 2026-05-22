#include "stdafx.h"
#include "ParticlePass.h"
#include "engine/Runtime/GpuResource/UploadBuffer.h"
#include "engine/Functions/Shader/ShaderReflection.h"
#include "engine/Functions/ECS/Component/TransformComponent.h"

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

	auto& rootIndex = RootSignatureBuilder::GetRootIndexMap("Renderer : particle PSO");
	auto view = registry.View<Component::ParticleEmitterComponent>();


	size_t baseIndex = 0;
	for (auto entity : view) {
		auto* emitter = registry.GetComponent<Component::ParticleEmitterComponent>(entity);
		if (!emitter->active || emitter->particles.empty()) continue;

		// GPUへ行列を転送
		UploadMatrices(gfx, emitter->particles, registry, baseIndex, emitter->isBillboard);

		gfx.SetPipelineState(renderCtx->GetGraphicsPSO("Renderer : particle PSO"));
		gfx.SetRootSignature(renderCtx->GetRootSignature("Renderer : particle PSO"));
		gfx.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		gfx.SetVertexBuffer(0, vertex_.VertexBufferView());
		gfx.SetIndexBuffer(index_.IndexBufferView());

		// カメラ
		gfx.SetDynamicConstantBufferView(rootIndex["gCameraMatrix"], sizeof(Component::CameraForGPU), &camera_->forGPU);
		// baseIndex
		_declspec(align(16)) struct {
			uint32_t index;
			uint32_t pad[3];
		}indexConstant;
		indexConstant.index = (uint32_t)baseIndex;
		gfx.SetDynamicConstantBufferView(rootIndex["gBaseIndex"], sizeof(uint32_t), &indexConstant);

		// ワールド行列
		gfx.SetDynamicDescriptor(
			rootIndex["gWorldMatrices"],
			0,
			worldMatrixBuffer_.GetSRV()
		);

		_declspec(align(16)) struct {
			Math::Color color;
		}constants;
		constants.color = Math::Color::WHITE;
		gfx.SetDynamicConstantBufferView(rootIndex["gMaterial"], sizeof(constants), &constants);
		gfx.SetDynamicDescriptor(rootIndex["gTexture"], 0, emitter->texture.GetSRV());

		gfx.DrawIndexedInstanced(6, static_cast<UINT>(emitter->particles.size()), 0, 0, static_cast<UINT>(baseIndex));
		baseIndex += emitter->particles.size();
	}
	particleCount_ = 0;
	matrices_.clear();
}

void ParticlePass::UploadMatrices(GraphicsContext& gfx, std::vector<Component::Particle>& particles, ECS::Registry& registry, size_t baseIndex, bool isBillboard) {
	size_t remaining = maxParticles_ - baseIndex;
	particleCount_ += particles.size();
	size_t count = std::min(particleCount_, remaining);
	size_t currentCount = std::min(particles.size(), remaining);

	if (count == 0) return;

	matrices_.reserve(count);

	Math::Matrix4x4 billBoardMatrix{};
	if (isBillboard) {
		Math::Matrix4x4 backToFrontMatrix;
		backToFrontMatrix.MakeRotate(Math::Vector3::UP * PI);
		auto* transform = registry.GetComponent<Component::TransformComponent>(GetTargetCamera()->entity);
		billBoardMatrix = backToFrontMatrix * transform->MakeAffineMatrix4x4();
		billBoardMatrix.m[3][0] = 0.0f;
		billBoardMatrix.m[3][1] = 0.0f;
		billBoardMatrix.m[3][2] = 0.0f;
	}

	for (size_t i = 0; i < currentCount; ++i) {
		if (isBillboard) {
			Math::Matrix4x4 scaleMatrix;
			scaleMatrix.MakeScale(particles[i].transform.scale);
			Math::Matrix4x4 translateMatrix;
			translateMatrix.MakeTranslate(particles[i].transform.translate);
			Math::Matrix4x4 worldMatrix = scaleMatrix * billBoardMatrix * translateMatrix;
			matrices_.push_back(worldMatrix);
		} else {
			matrices_.push_back(particles[i].transform.MakeAffineMatrix4x4());
		}
	}
	size_t size = count * sizeof(Math::Matrix4x4);
	memcpy(worldMatrixUpload_.Map(), matrices_.data(), size);
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
	size_t bufferSize = maxParticles * sizeof(Math::Matrix4x4);

	worldMatrixBuffer_.Create(
		L"ParticleWorldMatrices",
		static_cast<uint32_t>(maxParticles),
		sizeof(Math::Matrix4x4)
	);

	worldMatrixUpload_.Create(
		L"ParticleWorldMatricesUpload",
		bufferSize
	);

}

}
}