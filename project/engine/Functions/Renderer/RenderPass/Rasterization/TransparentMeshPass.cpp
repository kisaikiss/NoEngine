#include "TransparentMeshPass.h"

namespace NoEngine {
namespace Render {

void TransparentMeshPass::Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) {
	EnsurePSOsInitialized();

	Collect(registry);
	Sort();
	RenderItems(gfx, resourceRegistry, transparentItems_, registry); // 半透明のみ。不透明・アウトラインはMeshPassが既に描画済み
}

}
}