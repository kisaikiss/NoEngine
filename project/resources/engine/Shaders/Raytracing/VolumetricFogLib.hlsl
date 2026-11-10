RaytracingAccelerationStructure gScene : register(t0);
Texture2D<float> gDepth : register(t1);
RWTexture2D<float4> gFog : register(u0);

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

    if (gTraceShadows != 0)
    {
        RayDesc ray;
        ray.Origin = position;
        ray.Direction = directionToLight;
        ray.TMin = rayBias;
        ray.TMax = distanceToLight - rayBias;

        ShadowPayload payload;
        payload.occluded = true;
        // TLAS の表面シャドウと同じキャスターマスクを使う。
        TraceRay(gScene, RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER,
                 0x01, 0, 0, 0, ray, payload);
        if (payload.occluded)
            return 0.0;
    }

    float attenuation = pow(saturate(1.0 - distanceToLight / gSpotRange), gSpotDecay);
    const float isotropicPhase = 0.07957747;
    return gSpotRadiance * (attenuation * cone * isotropicPhase);
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

    float3 direction = normalize(Unproject(ndc, 0.0) - gCameraPosition);
    float end = gMaxDistance;
    if (depth < 1.0)
        end = min(end, length(Unproject(ndc, depth) - gCameraPosition));

    float stepLength = end / gStepCount;
    float3 scattering = 0.0;
    float transmittance = 1.0;
    for (uint i = 0; i < gStepCount; ++i)
    {
        float3 position = gCameraPosition + direction * ((i + 0.5) * stepLength);
        float height = max(position.y - gBaseHeight, 0.0);
        float extinction = gDensity * exp(-height * gHeightFalloff);
        float stepT = exp(-extinction * stepLength);
        float weight = transmittance * (1.0 - stepT) * gScatteringAlbedo;
        if (weight > 0.0)
            scattering += weight * (gAmbient + EvaluateSpot(position));
        transmittance *= stepT;
        if (transmittance < 1e-3)
            break;
    }

    gFog[pixel] = float4(scattering, transmittance);
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
