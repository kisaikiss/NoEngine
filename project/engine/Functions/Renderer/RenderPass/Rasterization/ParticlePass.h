#pragma once
#include "../RenderPass.h"
#include "engine/Runtime/GpuResource/GpuBuffer.h"
#include "engine/Math/MathInclude.h"
#include "engine/Functions/ECS/Component/ParticleEmitterComponent.h"
#include "engine/Functions/ECS/Component/CameraComponent.h"

namespace NoEngine {
namespace Render {

class ParticlePass : public RenderPass{
public:
	ParticlePass();
	void Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) override;
private:
	ByteAddressBuffer vertex_;
	ByteAddressBuffer index_;

	StructuredBuffer worldMatrixBuffer_;
	UploadBuffer worldMatrixUpload_;

	struct ParticleVertex {
		Math::Vector4 position;
		Math::Vector2 texcoord;
	};

	__declspec(align(16))struct ParticleForGPU {
		Math::Matrix4x4 worldMatrix;
		Math::Color color;
	};

	std::vector< Component::ParticleEmitterComponent*> emitters_;

	size_t maxParticles_;

	Component::CameraComponent* camera_;
	std::vector<ParticleForGPU> particleForGpu_;
	size_t particleCount_ = 0;

	void UploadMatrices(GraphicsContext& gfx, std::vector<Component::Particle>& particles, ECS::Registry& registry, size_t baseIndex, bool isBillboard = false);
	void Initialize(size_t maxParticles);
};

}
}
