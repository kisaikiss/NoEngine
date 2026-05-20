#pragma once
#include "../RenderPass.h"

namespace NoEngine {
namespace Render {
class BoxFilterPass : public RenderPass {
public:
	void Execute(GraphicsContext& gfx, ECS::Registry& registry) override;
};
}
}