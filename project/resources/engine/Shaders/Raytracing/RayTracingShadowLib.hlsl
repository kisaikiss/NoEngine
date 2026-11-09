#define MAX_SHADOW_LIGHTS 64

struct ShadowPayload
{
    bool occluded;
};

RaytracingAccelerationStructure gSceneTLAS : register(t0);

Texture2D<float4> gWorldPosTex : register(t1);
Texture2D<float4> gNormalTex : register(t3);

RWTexture2DArray<float> gShadowMask : register(u0);

struct LightNums
{
    int directionalLightNum;
    int pointLightNum;
    int spotLightNum;
};
ConstantBuffer<LightNums> gLightNums : register(b0);

struct DirectionalLight
{
    float4 color;
    float3 direction;
    float intensity;
    float shadowAngularRadiusDegrees;
    uint shadowSamples;
};
StructuredBuffer<DirectionalLight> gDirectionalLights : register(t2);

struct PointLight
{
    float4 color;
    float3 position;
    float intensity;
    float radius;
    float decay;
    float shadowRadius;
    uint shadowSamples;
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
    float shadowRadius;
    uint shadowSamples;
};
StructuredBuffer<SpotLight> gSpotLights : register(t5);

static const float gShadowMaxDistance = asfloat(0x7F7FFFFF);

static const uint kInstanceMaskShadowCasterBit = 0x01;
static const float kShadowRayMinDistance = 0.01f;

float TraceShadowRay(float3 origin, float3 direction, float tMax)
{
    RayDesc rayDesc;
    rayDesc.Origin = origin;
    rayDesc.Direction = direction;
    rayDesc.TMin = kShadowRayMinDistance;
    rayDesc.TMax = tMax;

    ShadowPayload payload;
    payload.occluded = true;

    uint rayFlags = RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER;
    TraceRay(gSceneTLAS, rayFlags, kInstanceMaskShadowCasterBit, 0, 1, 0, rayDesc, payload);

    return payload.occluded ? 0.3f : 1.0f;
}

uint HashShadowSample(uint seed)
{
    seed ^= seed >> 16;
    seed *= 0x7feb352du;
    seed ^= seed >> 15;
    seed *= 0x846ca68bu;
    return seed ^ (seed >> 16);
}

uint ShadowSampleSeed(uint2 pixel, uint slice)
{
    return pixel.x * 0x9e3779b9u ^ pixel.y * 0x85ebca6bu ^ slice * 0xc2b2ae35u;
}

float2 SampleShadowDisk(uint seed, uint sampleIndex)
{
    // 円盤の各象限から1点ずつ、画素ごとに異なる位置を選ぶ。
    uint random = HashShadowSample(seed + (sampleIndex + 1u) * 0x27d4eb2du);
    float2 jitter = (float2(random & 0xffffu, random >> 16) + 0.5f) / 65536.0f;
    float angle = (sampleIndex + jitter.x) * 1.57079632679f;
    return float2(cos(angle), sin(angle)) * sqrt(jitter.y);
}

void BuildShadowDiskBasis(float3 diskNormal, out float3 tangent, out float3 bitangent)
{
    float3 basisAxis = abs(diskNormal.y) < 0.99f ? float3(0, 1, 0) : float3(1, 0, 0);
    tangent = normalize(cross(basisAxis, diskNormal));
    bitangent = cross(diskNormal, tangent);
}

float TraceDirectionalShadow(float3 origin, DirectionalLight light, uint2 pixel, uint slice)
{
    float3 toLight = -light.direction;
    if (light.shadowSamples != 4 || !(light.shadowAngularRadiusDegrees > 0.0f))
        return TraceShadowRay(origin, toLight, gShadowMaxDistance);

    float3 tangent, bitangent;
    BuildShadowDiskBasis(toLight, tangent, bitangent);
    float diskRadius = tan(radians(min(light.shadowAngularRadiusDegrees, 45.0f)));

    uint seed = ShadowSampleSeed(pixel, slice);
    float visibility = 0.0f;
    [unroll]
    for (uint sampleIndex = 0; sampleIndex < 4; ++sampleIndex)
    {
        float2 offset = SampleShadowDisk(seed, sampleIndex) * diskRadius;
        float3 direction = normalize(toLight + tangent * offset.x + bitangent * offset.y);
        visibility += TraceShadowRay(origin, direction, gShadowMaxDistance);
    }
    return visibility * 0.25f;
}

float TraceLocalShadow(float3 origin, float3 toLight, float distanceToLight, float3 diskNormal,
    float shadowRadius, uint shadowSamples, uint2 pixel, uint slice)
{
    if (shadowSamples != 4 || !(shadowRadius > 0.0f))
        return TraceShadowRay(origin, toLight / distanceToLight, distanceToLight);

    float3 tangent, bitangent;
    BuildShadowDiskBasis(normalize(diskNormal), tangent, bitangent);
    uint seed = ShadowSampleSeed(pixel, slice);
    float visibility = 0.0f;
    [unroll]
    for (uint sampleIndex = 0; sampleIndex < 4; ++sampleIndex)
    {
        float2 offset = SampleShadowDisk(seed, sampleIndex) * shadowRadius;
        float3 toSample = toLight + tangent * offset.x + bitangent * offset.y;
        float sampleDistance = length(toSample);
        // 各レイは光源円盤上のサンプル位置まで追跡する。
        if (sampleDistance > kShadowRayMinDistance)
            visibility += TraceShadowRay(origin, toSample / sampleDistance, sampleDistance);
        else
            visibility += 1.0f;
    }
    return visibility * 0.25f;
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

    float3 encodedNormal = gNormalTex.Load(int3(dispatchIdx, 0)).xyz;
    float3 normal = normalize(encodedNormal * 2.0f - 1.0f);
    const float offsetScale = 0.01f;
    float3 offsetPos = worldPos.xyz + (normal * offsetScale);

    uint slice = 0;

    // ディレクショナルライト（無限遠）
    for (int d = 0; d < gLightNums.directionalLightNum && slice < MAX_SHADOW_LIGHTS; ++d, ++slice)
    {
        gShadowMask[uint3(dispatchIdx, slice)] = TraceDirectionalShadow(offsetPos, gDirectionalLights[d], dispatchIdx, slice);
    }

    // ポイントライト（radiusを超えたら無条件で影なし＝トレース省略）
    for (int p = 0; p < gLightNums.pointLightNum && slice < MAX_SHADOW_LIGHTS; ++p, ++slice)
    {
        PointLight light = gPointLights[p];
        float3 toLight = light.position - offsetPos;
        float dist = length(toLight);

        if (dist <= kShadowRayMinDistance || dist > light.radius)
        {
            gShadowMask[uint3(dispatchIdx, slice)] = 1.0f;
            continue;
        }
        // ポイントライトは受け面から見た円盤で発光面を近似する。
        gShadowMask[uint3(dispatchIdx, slice)] = TraceLocalShadow(offsetPos, toLight, dist, toLight,
            light.shadowRadius, light.shadowSamples, dispatchIdx, slice);
    }

    // スポットライト（円錐外・distance超過は無条件で影なし）
    for (int s = 0; s < gLightNums.spotLightNum && slice < MAX_SHADOW_LIGHTS; ++s, ++slice)
    {
        SpotLight light = gSpotLights[s];
        float3 toLight = light.position - offsetPos;
        float dist = length(toLight);
        float3 dirToLight = toLight / max(dist, kShadowRayMinDistance);

        // Default_PS.hlsl と同じ向き規約（光→サーフェス方向とdirectionの内積）
        float cosAngle = dot(-dirToLight, light.direction);

        if (dist <= kShadowRayMinDistance || dist > light.distance || cosAngle < light.cosAngle)
        {
            gShadowMask[uint3(dispatchIdx, slice)] = 1.0f;
            continue;
        }
        // スポットライトの円盤は照射方向に向ける。
        gShadowMask[uint3(dispatchIdx, slice)] = TraceLocalShadow(offsetPos, toLight, dist, light.direction,
            light.shadowRadius, light.shadowSamples, dispatchIdx, slice);
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
