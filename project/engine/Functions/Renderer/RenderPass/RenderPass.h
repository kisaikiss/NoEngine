#pragma once
#include "engine/Runtime/Command/GraphicsContext.h"
#include "engine/Functions/ECS/Registry.h"
#include "../RenderContext.h"
#include "../RenderGraph.h"

namespace NoEngine {
namespace Render {
class RenderPass {
public:
	
	RenderPass() : renderContext_(nullptr) {}
	virtual ~RenderPass() = default;

	virtual void Setup(RenderGraphBuilder& builder) {
		for (const auto& input : inputs_) {
			builder.ReadTexture(input);
		}
		for (const auto& output : outputs_) {
			builder.WriteRenderTarget(output, autoClear_);
		}
	}

	virtual void Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) = 0;

	void AddInput(const std::string& input) { inputs_.emplace_back(input); }
	void AddOutput(const std::string& output) { outputs_.emplace_back(output); }
	void SetClearTarget(bool autoClear) { autoClear_ = autoClear; }
	void SetTargetCamera(const std::string& targetCamera) { targetCameraName_ = targetCamera; }

	void SetRenderContext(RenderContext* renderContext) { renderContext_ = renderContext; }
protected:
	Component::CameraComponent* GetTargetCamera() { return renderContext_->GetCamera(targetCameraName_); }
	RenderContext* GetRenderContext() { return renderContext_; }

private:
	std::vector<std::string> inputs_;
	std::vector<std::string> outputs_;
	std::string targetCameraName_;
	bool autoClear_ = false;
	bool depthClear_ = false;

	RenderContext* renderContext_;
};
}
}