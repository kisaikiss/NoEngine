#pragma once
#include "../RenderPass.h"
#include "engine/Assets/Texture/TextureManager.h"

namespace NoEngine {
namespace Render {
class DissolvePass : public RenderPass {
public:
	DissolvePass();
	void Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) override;
private:
	TextureRef maskTexture_;
	std::string loadedMaskTextureName_;
	uint64_t loadedMaskTextureGeneration_ = 0;
	float threshold_ = 0.5f;
};
}
}
