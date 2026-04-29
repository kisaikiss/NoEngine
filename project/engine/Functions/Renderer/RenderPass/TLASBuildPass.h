#pragma once
#include "RenderPass.h"

#include "../../../Assets/RaytracingMesh.h"

namespace NoEngine {
namespace Render {
class TLASBuildPass :
    public RenderPass {
public:
    void Execute(GraphicsContext& gfx, ECS::Registry& registry) override;
private:
    struct RaytracingInstance {
        D3D12_RAYTRACING_INSTANCE_DESC desc;
        RaytracingMesh* rtMesh;
    };
    std::vector<RaytracingInstance> instances_;

    void BuildRaytracingInstances(ECS::Registry& registry);
    void BuildTLAS(GraphicsContext& gfx);

    /// <summary>
    /// D3D12_RAYTRACING_INSTANCE_DESC のトランスフォーム情報を指定された Transform で設定する関数。
    /// </summary>
    /// <param name="instance">設定対象の D3D12_RAYTRACING_INSTANCE_DESC への参照。トランスフォーム情報が書き込まれる。</param>
    /// <param name="transform">コピー元の Transform。インスタンスの行列や位置・回転・スケール情報を提供する（const 参照）。</param>
    void FillInstanceTransform(D3D12_RAYTRACING_INSTANCE_DESC& instance, const Transform& transform);
};
}
}

