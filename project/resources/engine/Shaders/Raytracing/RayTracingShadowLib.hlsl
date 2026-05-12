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
    float3 gLightDir;
    float pad1;
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
static const float gShadowMaxDistance = asfloat(0x7F7FFFFF);


float3 GetWorldPosition(float2 uv, float z)
{
    // DirectXの標準的なNDC変換
    float x = uv.x * 2.0f - 1.0f;
    float y = 1.0f - uv.y * 2.0f; // ここでYを反転しつつ-1～1にする
    
    float4 clipPos = float4(x, y, z, 1.0f);
    float4 worldPos = mul(gInvViewProj, clipPos); // または mul(gInvViewProj, clipPos)
    
    return worldPos.xyz / worldPos.w;
}

[shader("raygeneration")]
void RayGen_Shadow()
{
    uint2 dispatchIdx = DispatchRaysIndex().xy;
    float2 dispatchDim = DispatchRaysDimensions().xy;
    
    float depth = gDepth.Load(int3(dispatchIdx, 0));
    if (depth >= 1.0f)
    {
        gShadowMask[dispatchIdx] = 1.0; // Not shadowed
        return;
    }

    float2 uv = (dispatchIdx + 0.5f) / dispatchDim;
    float3 worldPosition = GetWorldPosition(uv, depth);

    float3 lightDir = float3(0, -1, 0);
    RayDesc rayDesc;
    rayDesc.Origin = worldPosition;
    rayDesc.Direction = lightDir;
    rayDesc.TMin = 0.01f;
    rayDesc.TMax = gShadowMaxDistance;

    ShadowPayload payload;
    payload.occluded = false;

    TraceRay(gSceneTLAS, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, ~0, 0, 1, 0, rayDesc, payload);


    gShadowMask[dispatchIdx] = payload.occluded ? 0.0f : 1.0f;
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