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

	std::vector< Component::ParticleEmitterComponent*> emitters_;

	size_t maxParticles_;

	Component::CameraComponent* camera_;
	std::vector<Math::Matrix4x4> matrices_;
	size_t particleCount_ = 0;

	void UploadMatrices(GraphicsContext& gfx, std::vector<Component::Particle>& particles, size_t baseIndex);
	void Initialize(size_t maxParticles);
};

}
}
