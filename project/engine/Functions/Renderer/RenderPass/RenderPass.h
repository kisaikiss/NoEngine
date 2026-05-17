#pragma once
#include "engine/Runtime/Command/GraphicsContext.h"
#include "engine/Functions/ECS/Registry.h"
#include "../RenderContext.h"

namespace NoEngine {
namespace Render {
class RenderPass {
public:
	virtual ~RenderPass() = default;
	virtual void Execute(GraphicsContext& gfx, ECS::Registry& registry) = 0;
	virtual void DebugExecute(GraphicsContext& gfx, ECS::Registry& registry) {
		static_cast<void>(gfx);
		static_cast<void>(registry);
	}

	void SetRenderContext(RenderContext* renderContext) { renderContext_ = renderContext; }
protected:
	RenderContext* GetRenderContext() { return renderContext_; }

private:
	RenderContext* renderContext_;
};
}
}