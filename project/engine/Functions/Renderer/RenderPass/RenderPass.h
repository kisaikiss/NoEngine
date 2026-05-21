#pragma once
#include "engine/Runtime/Command/GraphicsContext.h"
#include "engine/Functions/ECS/Registry.h"
#include "../RenderContext.h"
#include "../RenderGraph.h"

namespace NoEngine {
namespace Render {
class RenderPass {
public:

	enum class TargetCameraType {
		kMain,
		kDebug,
		kCustom,
	};
	
	RenderPass() : renderContext_(nullptr) {}
	virtual ~RenderPass() = default;

	virtual void Setup(RenderGraphBuilder& builder) {
		for (const auto& input : inputs_) {
			builder.ReadTexture(input);
		}
		for (const auto& output : outputs_) {
			builder.WriteRenderTarget(output, autoClear_);
		}
		if (!depthOutput_.empty()) {
			builder.WriteDepthStencil(depthOutput_, clearDepth_);
		}
	}

	virtual void Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) = 0;

	void AddInput(const std::string& input) { inputs_.emplace_back(input); }
	void AddOutput(const std::string& output) { outputs_.emplace_back(output); }
	void SetDepthOutput(const std::string& depth, bool clearDepth = false) { depthOutput_ = depth; clearDepth_ = clearDepth; }
	void SetClearTarget(bool autoClear) { autoClear_ = autoClear; }
	void SetTargetCameraType(TargetCameraType type) { targetCameraType_ = type; }
	void SetTargetCamera(const std::string& targetCamera) { 
		targetCameraType_ = TargetCameraType::kCustom;
		targetCameraName_ = targetCamera;
	}

	void SetRenderContext(RenderContext* renderContext) { renderContext_ = renderContext; }
protected:
	Component::CameraComponent* GetTargetCamera() { 
		switch (targetCameraType_) {
		case NoEngine::Render::RenderPass::TargetCameraType::kMain:
			return renderContext_->GetCamera();
			break;
		case NoEngine::Render::RenderPass::TargetCameraType::kDebug:
			return renderContext_->GetDebugCamera();
			break;
		case NoEngine::Render::RenderPass::TargetCameraType::kCustom:
			return renderContext_->GetCamera(targetCameraName_);
			break;
		default:
			return renderContext_->GetCamera();
			break;
		}
	}
	RenderContext* GetRenderContext() { return renderContext_; }

private:
	std::vector<std::string> inputs_;
	std::vector<std::string> outputs_;
	std::string depthOutput_;
	TargetCameraType targetCameraType_ = TargetCameraType::kMain;
	std::string targetCameraName_ = "GameCamera";
	bool autoClear_ = false;
	bool clearDepth_ = false;

	RenderContext* renderContext_;
};
}
}