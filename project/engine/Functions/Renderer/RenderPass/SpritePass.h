//#pragma once
//#include "RenderPass.h"
//#include "engine/Functions/ECS/Component/SpriteComponent.h"
//#include "engine/Functions/ECS/Component/MaterialComponent.h"
//#include "engine/Functions/ECS/Component/Transfrom2DComponent.h"
//#include "engine/Runtime/GpuResource/GpuBuffer.h"
//#include "engine/Math/Types/Matrix4x4.h"
//
//namespace NoEngine {
//namespace Render {
//
//class SpritePass final :
//    public RenderPass {
//public:
//    SpritePass();
//    ~SpritePass();
//    void Execute(GraphicsContext& gfx, ECS::Registry& registry) override;
//private:
//    struct DrawItem {
//        Component::Transform2DComponent* transform;
//        Component::SpriteComponent* sprite;
//        Component::MaterialComponent* material;
//        GraphicsPSO* pso;
//    };
//
//    std::vector<DrawItem> items_;
//
//    struct Batch {
//        TextureRef texture;
//        GraphicsPSO* pso;
//        std::span<DrawItem> sprites;
//        uint32_t vertexCount;
//        ByteAddressBuffer vertexBuffer;
//    };
//
//    std::vector<Batch> batches_;
//
//    struct SpriteVertex {
//        Vector4 position; // ワールド座標系
//        Vector2 texcoord;
//    };
//    std::vector<SpriteVertex> tempVertices_;
//   
//
//    void Collect(ECS::Registry& registry);
//    void Sort();
//    void BuildBatches();
//    void BuildVertexBuffers(GraphicsContext& gfx);
//    void Render(GraphicsContext& gfx);
//
//
//    void BuildQuadVertices(
//        std::span<SpriteVertex> dst,
//        size_t& cursor,
//        const  Component::Transform2DComponent& tr,
//        const  Component::SpriteComponent& sprite,
//        const  Component::MaterialComponent& mat);
//
//    ByteAddressBuffer indexBuffer_;
//    D3D12_INDEX_BUFFER_VIEW indexBufferView_;
//
//};
//
//}
//}
