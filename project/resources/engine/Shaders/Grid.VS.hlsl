#include "Grid.hlsli"

GridVertex main(uint vertexID : SV_VertexID)
{
    const float2 corners[6] = {
        float2(-1, -1), float2(-1, 1), float2(1, -1),
        float2(1, -1), float2(-1, 1), float2(1, 1)
    };
    float2 p = centerRadius.xy + corners[vertexID] * centerRadius.z;
    float3 world;
    if (options.x < 0.5) world = float3(p.x, originSpacing.y, p.y);
    else if (options.x < 1.5) world = float3(p.x, p.y, originSpacing.z);
    else world = float3(originSpacing.x, p.x, p.y);
    GridVertex output;
    output.worldPosition = world;
    output.position = mul(float4(world, 1), viewProjection);
    // 通常の深度方向に合わせ、同一面との競合を小さな値で緩和する。
    output.position.z -= options.w * output.position.w;
    return output;
}
