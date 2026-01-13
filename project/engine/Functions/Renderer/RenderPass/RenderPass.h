#pragma once
#include "engine/Runtime/Command/GraphicsContext.h"
#include "engine/Functions/ECS/Registry.h"
#include "engine/Functions/Camera/CameraBase.h"

namespace NoEngine {
namespace Render {
class RenderPass {
public:
	virtual ~RenderPass() = default;
	virtual void Execute(GraphicsContext& gfx, ECS::Registry& registry) = 0;

	void SetCamera(CameraBase* camera) { camera_ = camera; }
protected:
	CameraBase* GetCamera() { return camera_; }
private:
	CameraBase* camera_;
};
}
}