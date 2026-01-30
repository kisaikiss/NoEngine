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

	void SetRenderContext(RenderContext* renderContext) { renderContext_ = renderContext; }
protected:
	RenderContext* GetRenderContext() { return renderContext_; }

	// ToDo : コンフリクト回避のための関数。チーム制作終了後削除
	const CameraBase* GetCamera() { return renderContext_->GetCamera(); }
private:
	RenderContext* renderContext_;
};
}
}