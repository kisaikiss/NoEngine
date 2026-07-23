#define MAX_SHADOW_LIGHTS 16

struct ShadowPayload
{
    bool occluded;
};

RaytracingAccelerationStructure gSceneTLAS : register(t0);

Texture2D<float4> gWorldPosTex : register(t1);
Texture2D<float4> gNormalTex : register(t3);

RWTexture2DArray<float> gShadowMask : register(u0);

cbuffer CameraCB : register(b0)
{
    float4x4 gInvViewProj;
    float3 gCameraPos;
    float pad0;
    float3 gLightDir;
    float pad1;
};

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
StructuredBuffer<DirectionalLight> gDirectionalLights : register(t2);

struct PointLight
{
    float4 color;
    float3 position;
    float intensity;
    float radius;
    float decay;
};
StructuredBuffer<PointLight> gPointLights : register(t4);

struct SpotLight
{
    float4 color;
    float3 position;
    float intensity;
    float3 direction;
    float distance;
    float decay;
    float cosAngle;
    float cosFalloffStart;
};
StructuredBuffer<SpotLight> gSpotLights : register(t5);

static const float gShadowMaxDistance = asfloat(0x7F7FFFFF);

static const uint kInstanceMaskShadowCasterBit = 0x01;

float TraceShadowRay(float3 origin, float3 direction, float tMax)
{
    RayDesc rayDesc;
    rayDesc.Origin = origin;
    rayDesc.Direction = direction;
    rayDesc.TMin = 0.01f;
    rayDesc.TMax = tMax;

    ShadowPayload payload;
    payload.occluded = true;

    uint rayFlags = RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER;
    TraceRay(gSceneTLAS, rayFlags, kInstanceMaskShadowCasterBit, 0, 1, 0, rayDesc, payload);

    return payload.occluded ? 0.3f : 1.0f;
}

[shader("raygeneration")]
void RayGen_Shadow()
{
    uint2 dispatchIdx = DispatchRaysIndex().xy;
    float4 worldPos = gWorldPosTex.Load(int3(dispatchIdx, 0));

    if (worldPos.w == 0.0f)
    {
        for (uint i = 0; i < MAX_SHADOW_LIGHTS; ++i)
            gShadowMask[uint3(dispatchIdx, i)] = 1.0f;
        return;
    }

    float3 normal = normalize(gNormalTex.Load(int3(dispatchIdx, 0)).xyz);
    const float offsetScale = 0.01f;
    float3 offsetPos = worldPos.xyz + (normal * offsetScale);

    uint slice = 0;

    // ディレクショナルライト（無限遠）
    for (int d = 0; d < gLightNums.directionalLightNum && slice < MAX_SHADOW_LIGHTS; ++d, ++slice)
    {
        gShadowMask[uint3(dispatchIdx, slice)] = TraceShadowRay(offsetPos, gLightDir, gShadowMaxDistance);
    }

    // ポイントライト（radiusを超えたら無条件で影なし＝トレース省略）
    for (int p = 0; p < gLightNums.pointLightNum && slice < MAX_SHADOW_LIGHTS; ++p, ++slice)
    {
        PointLight light = gPointLights[p];
        float3 toLight = light.position - offsetPos;
        float dist = length(toLight);

        if (dist > light.radius)
        {
            gShadowMask[uint3(dispatchIdx, slice)] = 1.0f;
            continue;
        }
        gShadowMask[uint3(dispatchIdx, slice)] = TraceShadowRay(offsetPos, toLight / dist, dist);
    }

    // スポットライト（円錐外・distance超過は無条件で影なし）
    for (int s = 0; s < gLightNums.spotLightNum && slice < MAX_SHADOW_LIGHTS; ++s, ++slice)
    {
        SpotLight light = gSpotLights[s];
        float3 toLight = light.position - offsetPos;
        float dist = length(toLight);
        float3 dirToLight = toLight / dist;

        // Default_PS.hlsl と同じ向き規約（光→サーフェス方向とdirectionの内積）
        float cosAngle = dot(-dirToLight, light.direction);

        if (dist > light.distance || cosAngle < light.cosAngle)
        {
            gShadowMask[uint3(dispatchIdx, slice)] = 1.0f;
            continue;
        }
        gShadowMask[uint3(dispatchIdx, slice)] = TraceShadowRay(offsetPos, dirToLight, dist);
    }
}

[shader("miss")]
void Miss_Shadow(inout ShadowPayload payload)
{
    payload.occluded = false;
}

[shader("closesthit")]
void ClosestHit_Shadow(inout ShadowPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
}