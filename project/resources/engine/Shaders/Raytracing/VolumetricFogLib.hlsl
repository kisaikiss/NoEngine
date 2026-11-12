RaytracingAccelerationStructure gScene : register(t0);
Texture2D<float> gDepth : register(t1);
RWTexture2D<float4> gFog : register(u0);

struct DirectionalLight
{
    float4 color;
    float3 direction;
    float intensity;
    float shadowAngularRadiusDegrees;
    uint shadowSamples;
};
StructuredBuffer<DirectionalLight> gDirectionalLights : register(t2);

cbuffer FogConstants : register(b0)
{
    row_major float4x4 gInverseViewProjection;
    float3 gCameraPosition;
    float gDensity;
    float gBaseHeight;
    float gHeightFalloff;
    float gMaxDistance;
    uint gStepCount;
    float3 gSpotPosition;
    float gSpotRange;
    float3 gSpotDirection;
    float gSpotCosOuter;
    float3 gSpotRadiance;
    float gSpotCosInner;
    float gSpotDecay;
    float gAmbient;
    float gScatteringAlbedo;
    uint gTraceShadows;
    row_major float4x4 gWorldToBox;
    float gBoxDensity;
    float gBoxEdgeFade;
    float gBoxNoiseFrequency;
    float gBoxNoiseStrength;
    float3 gBoxNoiseOffset;
    uint gDirectionalLightCount;
};

struct ShadowPayload
{
    bool occluded;
};

float3 Unproject(float2 ndc, float depth)
{
    float4 world = mul(float4(ndc, depth, 1.0), gInverseViewProjection);
    return world.xyz / world.w;
}

bool IsFogLightOccluded(float3 position, float3 directionToLight, float maxDistance)
{
    if (gTraceShadows == 0)
        return false;

    RayDesc ray;
    ray.Origin = position;
    ray.Direction = directionToLight;
    ray.TMin = 0.01;
    ray.TMax = maxDistance;

    ShadowPayload payload;
    payload.occluded = true;
    // TLAS の表面シャドウと同じキャスターマスクを使う。
    TraceRay(gScene, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER,
             0x01, 0, 0, 0, ray, payload);
    return payload.occluded;
}

float3 EvaluateDirectional(float3 position)
{
    float3 scattering = 0.0;
    const float isotropicPhase = 0.07957747;
    for (uint i = 0; i < gDirectionalLightCount; ++i)
    {
        DirectionalLight light = gDirectionalLights[i];
        float directionLengthSquared = dot(light.direction, light.direction);
        if (!(light.intensity > 0.0) || !(directionLengthSquared > 0.0))
            continue;

        // 光の進行方向の逆へ飛ばし、視線の最大距離より遠い遮蔽物も判定する。
        float3 toLight = -light.direction * rsqrt(directionLengthSquared);
        if (!IsFogLightOccluded(position, toLight, asfloat(0x7F7FFFFF)))
            scattering += light.color.rgb * light.intensity * isotropicPhase;
    }
    return scattering;
}

float3 EvaluateSpot(float3 position)
{
    const float rayBias = 0.01;
    if (gSpotRange <= 0.0)
        return 0.0;

    float3 toLight = gSpotPosition - position;
    float distanceToLight = length(toLight);
    if (distanceToLight >= gSpotRange || distanceToLight <= 2.0 * rayBias)
        return 0.0;

    float3 directionToLight = toLight / distanceToLight;
    float cone = saturate((dot(-directionToLight, gSpotDirection) - gSpotCosOuter) /
                          max(gSpotCosInner - gSpotCosOuter, 1e-4));
    if (cone <= 0.0)
        return 0.0;

    if (IsFogLightOccluded(position, directionToLight, distanceToLight - rayBias))
        return 0.0;

    float attenuation = pow(saturate(1.0 - distanceToLight / gSpotRange), gSpotDecay);
    const float isotropicPhase = 0.07957747;
    return gSpotRadiance * (attenuation * cone * isotropicPhase);
}

struct FogRay
{
    float3 origin;
    float3 direction;
    float3 boxOrigin;
    float3 boxDirection;
    float end;
};

bool IntersectFogBox(FogRay ray, out float2 interval)
{
    interval = float2(0.0, ray.end);
    for (uint axis = 0; axis < 3; ++axis)
    {
        float origin = ray.boxOrigin[axis];
        float direction = ray.boxDirection[axis];
        if (direction == 0.0)
        {
            if (abs(origin) > 0.5)
                return false;
            continue;
        }

        float entry = (-0.5 - origin) / direction;
        float exit = (0.5 - origin) / direction;
        interval.x = max(interval.x, min(entry, exit));
        interval.y = min(interval.y, max(entry, exit));
        if (interval.x >= interval.y)
            return false;
    }
    return true;
}

float FogNoiseCorner(int3 cell)
{
    // C++ 側の風による移動量と同じ 256 格子の周期を使う。
    uint3 grid = asuint(cell) & 255u;
    uint hash = grid.x * 0x8da6b343u ^ grid.y * 0xd8163841u ^ grid.z * 0xcb1ab31fu;
    hash ^= hash >> 16;
    hash *= 0x7feb352du;
    hash ^= hash >> 15;
    hash *= 0x846ca68bu;
    hash ^= hash >> 16;
    return (hash >> 8) * (1.0 / 16777216.0);
}

float FogValueNoise(float3 position)
{
    int3 cell = int3(floor(position));
    float3 blend = frac(position);
    blend = blend * blend * (3.0 - 2.0 * blend);

    float xy00 = lerp(FogNoiseCorner(cell), FogNoiseCorner(cell + int3(1, 0, 0)), blend.x);
    float xy10 = lerp(FogNoiseCorner(cell + int3(0, 1, 0)), FogNoiseCorner(cell + int3(1, 1, 0)), blend.x);
    float xy01 = lerp(FogNoiseCorner(cell + int3(0, 0, 1)), FogNoiseCorner(cell + int3(1, 0, 1)), blend.x);
    float xy11 = lerp(FogNoiseCorner(cell + int3(0, 1, 1)), FogNoiseCorner(cell + int3(1, 1, 1)), blend.x);
    return lerp(lerp(xy00, xy10, blend.y), lerp(xy01, xy11, blend.y), blend.z);
}

void IntegrateInterval(FogRay ray, float2 interval, bool insideBox, inout float4 fog)
{
    float intervalLength = interval.y - interval.x;
    if (intervalLength <= 0.0 || fog.a < 1e-3)
        return;

    // Box 内は短い区間にもステップを割り当て、薄い霧を飛び越えないようにする。
    uint steps = insideBox ? gStepCount : max(1u, (uint)ceil(gStepCount * (intervalLength / ray.end)));
    float stepLength = intervalLength / steps;
    for (uint i = 0; i < steps; ++i)
    {
        float t = interval.x + (i + 0.5) * stepLength;
        float3 position = ray.origin + ray.direction * t;
        float height = max(position.y - gBaseHeight, 0.0);
        float extinction = gDensity * exp(-height * gHeightFalloff);
        if (insideBox)
        {
            float3 boxPosition = ray.boxOrigin + ray.boxDirection * t;
            float3 toFace = 0.5 - abs(boxPosition);
            float nearestFace = min(toFace.x, min(toFace.y, toFace.z));
            float fade = gBoxEdgeFade > 0.0 ? smoothstep(0.0, gBoxEdgeFade, nearestFace) : 1.0;
            float variation = 1.0;
            if (gBoxNoiseStrength > 0.0 && fade > 0.0)
            {
                float3 noisePosition = boxPosition * gBoxNoiseFrequency - gBoxNoiseOffset;
                variation += gBoxNoiseStrength * (2.0 * FogValueNoise(noisePosition) - 1.0);
            }
            extinction += gBoxDensity * fade * variation;
        }

        float stepT = exp(-extinction * stepLength);
        float weight = fog.a * (1.0 - stepT) * gScatteringAlbedo;
        if (weight > 0.0)
            fog.rgb += weight * (gAmbient + EvaluateDirectional(position) + EvaluateSpot(position));
        fog.a *= stepT;
        if (fog.a < 1e-3)
            break;
    }
}

[shader("raygeneration")]
void RayGen_Fog()
{
    uint2 pixel = DispatchRaysIndex().xy;
    float2 uv = (float2(pixel) + 0.5) / float2(DispatchRaysDimensions().xy);
    float2 ndc = uv * float2(2.0, -2.0) + float2(-1.0, 1.0);
    uint2 depthSize;
    gDepth.GetDimensions(depthSize.x, depthSize.y);
    float depth = gDepth.Load(int3(min(uint2(uv * depthSize), depthSize - 1), 0));

    FogRay ray;
    // 深度の異なる2点から、描画時の投影に一致する視線を復元する。
    float3 nearPosition = Unproject(ndc, 0.0);
    ray.direction = normalize(Unproject(ndc, 0.5) - nearPosition);
    ray.origin = nearPosition + ray.direction * dot(gCameraPosition - nearPosition, ray.direction);
    ray.end = gMaxDistance;
    if (depth < 1.0)
        ray.end = min(ray.end, max(0.0, dot(Unproject(ndc, depth) - ray.origin, ray.direction)));

    ray.boxOrigin = mul(float4(ray.origin, 1.0), gWorldToBox).xyz;
    // ローカル方向の長さを保ち、交差距離 t をワールド距離として使う。
    ray.boxDirection = mul(float4(ray.direction, 0.0), gWorldToBox).xyz;
    float4 fog = float4(0.0, 0.0, 0.0, 1.0);
    float2 boxInterval;
    if (gBoxDensity > 0.0 && IntersectFogBox(ray, boxInterval))
    {
        if (gDensity > 0.0)
            IntegrateInterval(ray, float2(0.0, boxInterval.x), false, fog);
        IntegrateInterval(ray, boxInterval, true, fog);
        if (gDensity > 0.0)
            IntegrateInterval(ray, float2(boxInterval.y, ray.end), false, fog);
    }
    else if (gDensity > 0.0)
        IntegrateInterval(ray, float2(0.0, ray.end), false, fog);

    gFog[pixel] = fog;
}

[shader("miss")]
void Miss_Fog(inout ShadowPayload payload)
{
    payload.occluded = false;
}

[shader("closesthit")]
void ClosestHit_Fog(inout ShadowPayload payload, in BuiltInTriangleIntersectionAttributes attributes)
{
    payload.occluded = true;
}
