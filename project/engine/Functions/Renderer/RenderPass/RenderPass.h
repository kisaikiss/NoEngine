#pragma once
#include "engine/Runtime/Command/GraphicsContext.h"
#include "engine/Functions/ECS/Registry.h"
#include "../RenderContext.h"
#include "../RenderGraph.h"

namespace NoEngine {
namespace Render {
class RenderPass {
public:
	
	RenderPass() : input_("ShadowMap"), output_("MainColor"), renderContext_(nullptr) {}
	virtual ~RenderPass() = default;
	virtual void Setup(RenderGraphBuilder& builder) {
		builder.ReadTexture(input_);
		builder.WriteRenderTarget(output_);
	}
	virtual void Execute(GraphicsContext& gfx, ECS::Registry& registry) = 0;

	void SetInput(const std::string& input) { input_ = input; }
	void SetOutput(const std::string& output) { output_ = output; }

	void SetRenderContext(RenderContext* renderContext) { renderContext_ = renderContext; }
protected:
	RenderContext* GetRenderContext() { return renderContext_; }

private:
	std::string input_;
	std::string output_;

	RenderContext* renderContext_;
};
}
}