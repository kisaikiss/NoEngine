#pragma once
#include "MeshPass.h"
namespace NoEngine {
namespace Render {
class DebugRenderPass :
    public MeshPass {
public:
    void Execute(GraphicsContext& gfx, ECS::Registry& registry) override;
private:
    /// <summary>
    /// DrawItemを収集します
    /// </summary>
    /// <param name="registry">ECSレジストリ</param>
    void Collect(ECS::Registry& registry) override;
};
}
}
