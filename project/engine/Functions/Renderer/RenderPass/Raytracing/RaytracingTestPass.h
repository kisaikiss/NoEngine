#pragma once
#include "../RenderPass.h"
namespace NoEngine {
namespace Render {
class RaytracingTestPass :
    public RenderPass {
    Component::CameraComponent* camera_ = nullptr;
    Math::Quaternion cameraRotate_;
public:
    void Execute(GraphicsContext& gfx, const RenderGraphRegistry& resourceRegistry, ECS::Registry& registry) override;
};
}
}
