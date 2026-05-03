// RayTracingShadowLib.hlsl

struct ShadowPayload
{
    bool occluded;
};

RaytracingAccelerationStructure gSceneTLAS : register(t0);

// Scene SRV テーブル
Texture2D<float> gDepth : register(t1);

// UAV テーブル
RWTexture2D<float> gShadowMask : register(u0);

// Camera
cbuffer CameraCB : register(b0)
{
    float4x4 gInvViewProj;
    float3 gCameraPos;
    float pad0;
};

// Light list
struct LightNums
{
    int directionalLightNum;
    int pointLightNum;
    int spotLightNum;
};
ConstantBuffer<LightNums> gLightNums : register(b1);

struct DirectionalLight
{
    float4 color;
    float3 direction;
    float intensity;
};
StructuredBuffer<DirectionalLight> gLights : register(t2);

// ShadowRay の最大距離
static const float gShadowMaxDistance = 1000.0f;


[shader("raygeneration")]
void RayGen_Shadow()
{
    uint2 pixel = DispatchRaysIndex().xy;
    uint2 size = DispatchRaysDimensions().xy;

    float2 uv = (pixel + 0.5) / size;

    // Depth 読み取り
    float depth = gDepth.Load(int3(pixel, 0));
    if (depth >= 1.0f)
    {
        gShadowMask[pixel] = 1.0f; // 遠平面 → 非影
        return;
    }

    // NDC → World
    float4 ndc = float4(uv * 2.0 - 1.0, depth, 1.0);
    float4 world = mul(ndc, gInvViewProj);
    world /= world.w;

    float3 worldPos = world.xyz;

    // 複数ライト対応：1つでも occluded なら影
    bool shadowed = false;

    for (uint i = 0; i < gLightNums.directionalLightNum; i++)
    {
        float3 lightDir = normalize(gLights[i].direction);

        RayDesc ray;
        ray.Origin = worldPos + lightDir * 0.01; // アーティファクト防止
        ray.Direction = lightDir;
        ray.TMin = 0.0;
        ray.TMax = gShadowMaxDistance;

        ShadowPayload payload;
        payload.occluded = false;

        TraceRay(
            gSceneTLAS,
            RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH,
            0xFF,
            0, 0, 0,
            ray,
            payload
        );

        if (payload.occluded)
        {
            shadowed = true;
            break; // 1つでも影なら影
        }
    }

    gShadowMask[pixel] = shadowed ? 0.0f : 1.0f;
}

[shader("miss")]
void Miss_Shadow(inout ShadowPayload payload)
{
    payload.occluded = false;
}

[shader("closesthit")]
void ClosestHit_Shadow(inout ShadowPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    payload.occluded = true;
}
