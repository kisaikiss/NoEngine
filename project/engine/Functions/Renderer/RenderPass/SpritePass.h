#pragma once
#include "RenderPass.h"
#include "engine/Functions/ECS/Component/SpriteComponent.h"
#include "engine/Functions/ECS/Component/Transform2DComponent.h"
#include "engine/Runtime/GpuResource/GpuBuffer.h"
#include "engine/Math/Types/Matrix4x4.h"
#pragma warning(push)
#pragma warning(disable: 4324)
namespace NoEngine {
namespace Render {

class SpritePass final :
    public RenderPass {
public:
    SpritePass();
    ~SpritePass();
    void Execute(GraphicsContext& gfx, ECS::Registry& registry) override;
private:
    struct DrawItem {
        Component::Transform2DComponent* transform;
        Component::SpriteComponent* sprite;
    };
    std::vector<DrawItem> items_;

    struct SpriteVertex {
        Vector4 position; // ワールド座標系
        Vector2 texcoord;
    };
    std::vector<SpriteVertex> vertices_;
    std::vector<uint16_t> indices_;
   
    void Collect(ECS::Registry& registry);
    void Sort();
    void MakeLocalQuad(const DrawItem& item , Vector2 out[4]);
    void GenerateVertices();
    void Render(GraphicsContext& gfx);
};

}
}
#pragma warning(pop)

