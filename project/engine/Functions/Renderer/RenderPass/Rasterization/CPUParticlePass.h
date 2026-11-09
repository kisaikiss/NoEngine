#pragma once
#include "../RenderPass.h"
#include "engine/Runtime/GpuResource/GpuBuffer.h"
#include "engine/Math/MathInclude.h"
#include "engine/Functions/ECS/Component/Effect/ParticleEmitterComponent.h"
#include "engine/Functions/ECS/Component/Common/CameraComponent.h"
#include "engine/Functions/ECS/Component/Common/TransformComponent.h"

namespace NoEngine {
namespace Render {

// Scene ビューだけが参照する、編集プレビュー専用の粒子。
struct EffectPreviewSource {
	std::weak_ptr<ECS::Registry> particles;
};

class CPUParticlePass : public RenderPass{
public:
	explicit CPUParticlePass(bool showEffectPreview = false);
	void Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) override;
private:
	ByteAddressBuffer vertex_;
	ByteAddressBuffer index_;
	ByteAddressBuffer ringVertex_;
	ByteAddressBuffer ringIndex_;
	ByteAddressBuffer cylinderVertex_;
	ByteAddressBuffer cylinderIndex_;
	uint32_t ringIndexCount_;
	uint32_t cylinderIndexCount_;

	StructuredBuffer worldMatrixBuffer_;
	UploadBuffer worldMatrixUpload_;

	struct ParticleVertex {
		Math::Vector4 position;
		Math::Vector2 texcoord;
	};

	struct DrawItem {
		Component::TransformComponent* transform;
		Component::ParticleComponent* particle;
		bool isPreview;
	};
	std::vector<DrawItem> items_;

	struct DrawBatch {
		Component::ParticleShape shape;
		TextureRef texture;
		uint32_t firstInstance;
		uint32_t instanceCount;
	};
	std::vector<DrawBatch> batches_;

	__declspec(align(16))struct ParticleForGPU {
		Math::Matrix4x4 worldMatrix;
		Math::Color color;
	};

	std::vector< Component::ParticleEmitterComponent*> emitters_;

	size_t maxParticles_;
	bool showEffectPreview_;

	Component::CameraComponent* camera_;
	std::vector<ParticleForGPU> particleForGpu_;

	void UploadMatrices(GraphicsContext& gfx, ECS::Registry& registry);
	void Initialize(size_t maxParticles);
	void GenerateRingMesh(std::vector<ParticleVertex>& outVerts, std::vector<uint32_t>& outIndices, uint32_t ringDivide = 32, float innerRadius = 0.2f, float outerRadius = 1.0f);

	void GenerateCylinderMesh(std::vector<ParticleVertex>& outVerts, std::vector<uint32_t>& outIndices,
		uint32_t cylinderDivide = 32, float topRadius = 1.f, float bottomRadius = 1.0f, float height = 3.0f);
};

}
}
