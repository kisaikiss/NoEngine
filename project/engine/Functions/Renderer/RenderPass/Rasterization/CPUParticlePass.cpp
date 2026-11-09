#include "stdafx.h"
#include "CPUParticlePass.h"
#include "engine/Runtime/GpuResource/UploadBuffer.h"
#include "engine/Functions/Shader/ShaderReflection.h"

namespace NoEngine {
namespace Render {

CPUParticlePass::CPUParticlePass() {
	{

		// --- シリンダーメッシュ生成 ---
		std::vector<ParticleVertex> cylinderVerts;
		std::vector<uint32_t> cylinderIndices;
		GenerateCylinderMesh(cylinderVerts, cylinderIndices);

		// Upload cylinder vertices
		UploadBuffer cylinderVertUpload;
		size_t vertSize = cylinderVerts.size() * sizeof(ParticleVertex);
		cylinderVertUpload.Create(L"CylinderVertexUpload", vertSize);
		memcpy(cylinderVertUpload.Map(), cylinderVerts.data(), vertSize);
		cylinderVertUpload.Unmap();
		cylinderVertex_.Create(L"CylinderVertex", static_cast<uint32_t>(cylinderVerts.size()), sizeof(ParticleVertex), cylinderVertUpload);

		// Upload cylinder indices
		UploadBuffer cylinderIdxUpload;
		size_t idxSize = cylinderIndices.size() * sizeof(uint32_t);
		cylinderIdxUpload.Create(L"CylinderIndexUpload", idxSize);
		memcpy(cylinderIdxUpload.Map(), cylinderIndices.data(), idxSize);
		cylinderIdxUpload.Unmap();
		cylinderIndex_.Create(L"CylinderIndex", static_cast<uint32_t>(cylinderIndices.size()), sizeof(uint32_t), cylinderIdxUpload);

		cylinderIndexCount_ = static_cast<uint32_t>(cylinderIndices.size());
	}

	// --- リングメッシュ生成 ---
	std::vector<ParticleVertex> ringVerts;
	std::vector<uint32_t> ringIndices;
	GenerateRingMesh(ringVerts, ringIndices);

	// Upload ring vertices
	UploadBuffer ringVertUpload;
	size_t vertSize = ringVerts.size() * sizeof(ParticleVertex);
	ringVertUpload.Create(L"RingVertexUpload", vertSize);
	memcpy(ringVertUpload.Map(), ringVerts.data(), vertSize);
	ringVertUpload.Unmap();
	ringVertex_.Create(L"ringVertex", static_cast<uint32_t>(ringVerts.size()), sizeof(ParticleVertex), ringVertUpload);

	// Upload ring indices
	UploadBuffer ringIdxUpload;
	size_t idxSize = ringIndices.size() * sizeof(uint32_t);
	ringIdxUpload.Create(L"RingIndexUpload", idxSize);
	memcpy(ringIdxUpload.Map(), ringIndices.data(), idxSize);
	ringIdxUpload.Unmap();
	ringIndex_.Create(L"ringIndex", static_cast<uint32_t>(ringIndices.size()), sizeof(uint32_t), ringIdxUpload);

	ringIndexCount_ = static_cast<uint32_t>(ringIndices.size());

	

	// Quad
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

void CPUParticlePass::Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) {
	static_cast<void>(resourceRegistry);
	auto* renderCtx = GetRenderContext();
	camera_ = GetTargetCamera();
	if (!camera_) return;
	auto& rootIndex = RootSignatureBuilder::GetRootIndexMap("Renderer : cpu particle PSO");

	// GPUへ行列を転送
	UploadMatrices(gfx, registry);

	if (batches_.empty()) return;

	gfx.SetPipelineState(renderCtx->GetGraphicsPSO("Renderer : cpu particle PSO"));
	gfx.SetRootSignature(renderCtx->GetRootSignature("Renderer : cpu particle PSO"));
	gfx.SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// カメラ
	gfx.SetDynamicConstantBufferView(rootIndex["gCameraMatrix"], sizeof(Component::CameraForGPU), &camera_->forGPU);

	// ワールド行列
	gfx.SetDynamicDescriptor(
		rootIndex["gWorldMatrices"],
		0,
		worldMatrixBuffer_.GetSRV()
	);
	for (const auto& batch : batches_) {
		// 頂点シェーダーでバッチの先頭位置を加算する。
		_declspec(align(16)) struct {
			uint32_t index;
			uint32_t pad[3];
		} indexConstant{ batch.firstInstance, {} };
		gfx.SetDynamicConstantBufferView(rootIndex["gBaseIndex"], sizeof(indexConstant), &indexConstant);

		gfx.SetDynamicDescriptor(rootIndex["gTexture"], 0, batch.texture.GetSRV());
		switch (batch.shape) {
		case NoEngine::Component::ParticleShape::kPlane:
			gfx.SetVertexBuffer(0, vertex_.VertexBufferView());
			gfx.SetIndexBuffer(index_.IndexBufferView());
			gfx.DrawIndexedInstanced(6, batch.instanceCount, 0, 0, 0);
			break;
		case NoEngine::Component::ParticleShape::kRing:
			gfx.SetVertexBuffer(0, ringVertex_.VertexBufferView());
			gfx.SetIndexBuffer(ringIndex_.IndexBufferView());
			gfx.DrawIndexedInstanced(ringIndexCount_, batch.instanceCount, 0, 0, 0);
			break;
		case NoEngine::Component::ParticleShape::kCylinder:
			gfx.SetVertexBuffer(0, cylinderVertex_.VertexBufferView());
			gfx.SetIndexBuffer(cylinderIndex_.IndexBufferView());
			gfx.DrawIndexedInstanced(cylinderIndexCount_, batch.instanceCount, 0, 0, 0);
			break;
		}
	}
}

void CPUParticlePass::UploadMatrices(GraphicsContext& gfx, ECS::Registry& registry) {
	items_.clear();
	batches_.clear();
	particleForGpu_.clear();
	auto view = registry.View<Component::ParticleComponent, Component::TransformComponent>();

	Math::Matrix4x4 billBoardMatrix{};
	{
		Math::Matrix4x4 backToFrontMatrix;
		backToFrontMatrix.MakeRotate(Math::Vector3::UP * PI);
		auto* transform = registry.GetComponent<Component::TransformComponent>(GetTargetCamera()->entity);
		if (!transform) return;
		billBoardMatrix = backToFrontMatrix * transform->MakeAffineMatrix4x4(registry);
		billBoardMatrix.m[3][0] = 0.0f;
		billBoardMatrix.m[3][1] = 0.0f;
		billBoardMatrix.m[3][2] = 0.0f;
	}

	// 描画対象の粒子を集める。
	for (auto entity : view) {
		auto* particle = registry.GetComponent<Component::ParticleComponent>(entity);
		auto* transform = registry.GetComponent<Component::TransformComponent>(entity);

		items_.push_back({ transform,particle });
	}

	if (items_.empty()) return;

	// 形状とテクスチャが同じ粒子をまとめる。
	std::sort(items_.begin(), items_.end(), [](const DrawItem& a, const DrawItem& b) {
		if (a.particle->shape != b.particle->shape) return a.particle->shape < b.particle->shape;
		return a.particle->texture < b.particle->texture;
		});

	if (items_.size() > maxParticles_) {
		items_.resize(maxParticles_);
	}
	for (const auto& item : items_) {
		const auto& particle = *item.particle;
		if (batches_.empty() || batches_.back().shape != particle.shape || batches_.back().texture != particle.texture) {
			batches_.push_back({ particle.shape, particle.texture, static_cast<uint32_t>(particleForGpu_.size()), 0 });
		}
		++batches_.back().instanceCount;

		Math::Matrix4x4 scaleMatrix;
		scaleMatrix.MakeScale(item.transform->scale);
		Math::Matrix4x4 translateMatrix;
		translateMatrix.MakeTranslate(item.transform->translate);
		Math::Matrix4x4 rotateMatrix;
		rotateMatrix.MakeRotate(item.transform->rotation);
		Math::Matrix4x4 worldMatrix = scaleMatrix * rotateMatrix * billBoardMatrix * translateMatrix;
		particleForGpu_.push_back({ worldMatrix, particle.color });
	}

	size_t size = particleForGpu_.size() * sizeof(ParticleForGPU);
	memcpy(worldMatrixUpload_.Map(), particleForGpu_.data(), size);
	worldMatrixUpload_.Unmap();
	gfx.CopyBufferRegion(
		worldMatrixBuffer_,
		0,
		worldMatrixUpload_,
		0,
		size
	);
	gfx.TransitionResource(worldMatrixBuffer_, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
}

void CPUParticlePass::Initialize(size_t maxParticles) {
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

void CPUParticlePass::GenerateRingMesh(std::vector<ParticleVertex>& outVerts, std::vector<uint32_t>& outIndices, uint32_t ringDivide, float innerRadius, float outerRadius) {
	outVerts.clear();
	outIndices.clear();
	outVerts.reserve(ringDivide * 4);
	outIndices.reserve(ringDivide * 6);

	const float kRadianPerDivide = 2.0f * PI / float(ringDivide);
	for (uint32_t index = 0; index < ringDivide; ++index) {
		const float kAngle = static_cast<float>(index) * kRadianPerDivide;
		const float kAngleNext = static_cast<float>(index + 1) * kRadianPerDivide;

		float sin = std::sinf(kAngle);
		float cos = std::cosf(kAngle);
		float sinNext = std::sinf(kAngleNext);
		float cosNext = std::cosf(kAngleNext);
		float u = static_cast<float>(index) / static_cast<float>(ringDivide);
		float uNext = static_cast<float>(index + 1) / static_cast<float>(ringDivide);
		outVerts.push_back({ {-sin * outerRadius, cos * outerRadius, 0.0f, 1.0f},{u,0.0f} });
		outVerts.push_back({ {-sinNext * outerRadius, cosNext * outerRadius, 0.0f, 1.0f},{uNext,0.0f} });
		outVerts.push_back({ {-sin * innerRadius, cos * innerRadius, 0.0f, 1.0f},{u,1.0f} });
		outVerts.push_back({ {-sinNext * innerRadius, cosNext * innerRadius, 0.0f, 1.0f},{uNext,1.0f} });

		uint32_t base = index * 4;
		// 三角形 1
		outIndices.push_back(base + 1);
		outIndices.push_back(base + 2);
		outIndices.push_back(base + 0);

		// 三角形 2: inner current, inner next, outer next
		outIndices.push_back(base + 1);
		outIndices.push_back(base + 3);
		outIndices.push_back(base + 2);

	}
}

void CPUParticlePass::GenerateCylinderMesh(std::vector<ParticleVertex>& outVerts, std::vector<uint32_t>& outIndices,
	uint32_t cylinderDivide, float topRadius, float bottomRadius, float height) {

	const float radianParDivide = 2.0f * PI / static_cast<float>(cylinderDivide);

	for (uint32_t index = 0; index < cylinderDivide; ++index) {
		float sin = std::sinf(static_cast<float>(index) * radianParDivide);
		float cos = std::cosf(static_cast<float>(index) * radianParDivide);
		float sinNext = std::sinf(static_cast<float>(index + 1) * radianParDivide);
		float cosNext = std::cosf(static_cast<float>(index + 1) * radianParDivide);
		float u = static_cast<float>(index) / static_cast<float>(cylinderDivide);
		float uNext = static_cast<float>(index + 1) / static_cast<float>(cylinderDivide);
		outVerts.push_back({ {-sin * topRadius, height, cos * topRadius, 1.0f}, {u,0.0f} });
		outVerts.push_back({ {-sinNext * topRadius,height ,cosNext * topRadius, 1.0f},{uNext,0.0f} });
		outVerts.push_back({ {-sin * bottomRadius,0.0f, cos * bottomRadius, 1.0f},{u,1.0f} });
		outVerts.push_back({ {-sinNext * bottomRadius,  0.0f, cosNext * bottomRadius,1.0f},{uNext,1.0f} });

		uint32_t base = index * 4;
		// 三角形 1
		outIndices.push_back(base + 1);
		outIndices.push_back(base + 2);
		outIndices.push_back(base + 0);

		// 三角形 2: inner current, inner next, outer next
		outIndices.push_back(base + 1);
		outIndices.push_back(base + 3);
		outIndices.push_back(base + 2);

	}
}

}
}
