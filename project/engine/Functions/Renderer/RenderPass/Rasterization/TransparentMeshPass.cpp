#include "TransparentMeshPass.h"

namespace NoEngine {
namespace Render {

void TransparentMeshPass::Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) {
	const auto* camera = GetTargetCamera();
	if (!camera) return;
	EnsurePSOsInitialized();

	const auto& items = GetRenderContext()->GetMeshDrawLists(registry, *camera).transparent;
	RenderItems(gfx, resourceRegistry, items, *camera, registry);
}

}
}
