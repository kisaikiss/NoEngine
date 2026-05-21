#pragma once
#include "../RenderPass.h"

#include "engine/Runtime/GpuResource/GpuBuffer.h"
#include "engine/Math/MathInclude.h"
#include "engine/Runtime/PipelineStateObject/GraphicsPSO.h"
#include "engine/Runtime/PipelineStateObject/RootSignature.h"
#include "engine/Assets/Texture/TextureManager.h"

namespace NoEngine {
namespace Render {

class SkyBoxPass :
    public RenderPass {
public:
    SkyBoxPass();
    void Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) override;
private:
    ByteAddressBuffer vertex_;
    ByteAddressBuffer index_;

    GraphicsPSO pso_;
    RootSignature rootSig_;
    TextureRef texture_;

    Component::CameraComponent* camera_;

    void InitVertices();
    void InitPSO();
};
}
}

