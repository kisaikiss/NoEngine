#pragma once
#include "../RenderPass.h"
#include "engine/Runtime/GpuResource/GpuBuffer.h"
#include "engine/Assets/Texture/TextureManager.h"

namespace NoEngine {

namespace Render {
class ParticlePass : public RenderPass {
public:
	ParticlePass();
	void Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) override;
private:

	ByteAddressBuffer vertex_;
	ByteAddressBuffer index_;
	Component::CameraComponent* camera_;
	TextureRef texture_;
};
}
}