#include "BackgroundCommon.hlsli"

// ワールド空間におけるプロシージャルな夜空。
// 空は無限の彼方にあるため、カメラの平行移動によって星の位置は変わらないが、カメラの回転によって視線ベクトルは変化する。
float Hash21(float2 p)
{
    p = frac(p * float2(123.34f, 456.21f));
    p += dot(p, p + 45.32f);
    return frac(p.x * p.y);
}

float3 SkyRay(float2 fragCoord)
{
    float fov = (iCameraParams.x > 0.0f) ? iCameraParams.x : 1.0471976f;
    float aspect = (iCameraParams.y > 0.0f) ? iCameraParams.y : iResolution.x / iResolution.y;
    float2 ndc = 2.0f * fragCoord / iResolution - 1.0f;
    float tanHalfFov = tan(0.5f * fov);
    float3 localRay = normalize(float3(ndc.x * tanHalfFov * aspect, ndc.y * tanHalfFov, 1.0f));
    return normalize(iCameraRight.xyz * localRay.x
                   + iCameraUp.xyz * localRay.y
                   + iCameraForward.xyz * localRay.z);
}

float StarLayer(float3 direction, float scale, float threshold, float size)
{
    float2 p = direction.xy / max(0.12f, direction.z + 1.15f) * scale;
    float2 cell = floor(p);
    float2 local = frac(p) - 0.5f;
    float seed = Hash21(cell);
    float star = 1.0f - smoothstep(0.0f, size, length(local - (float2(Hash21(cell + 7.1f), Hash21(cell + 19.7f)) - 0.5f) * 0.65f));
    float twinkle = 0.82f + 0.18f * sin(iTime * (1.1f + seed * 2.0f) + seed * 40.0f);
    return star * step(threshold, seed) * twinkle;
}

float HashGrid(float2 p)
{
    return frac(sin(dot(p, float2(127.1f, 311.7f))) * 43758.5453f);
}

float ValueNoise(float2 p)
{
    float2 cell = floor(p);
    float2 f = frac(p);
    f = f * f * (3.0f - 2.0f * f);
    float a = HashGrid(cell);
    float b = HashGrid(cell + float2(1.0f, 0.0f));
    float c = HashGrid(cell + float2(0.0f, 1.0f));
    float d = HashGrid(cell + float2(1.0f, 1.0f));
    return lerp(lerp(a, b, f.x), lerp(c, d, f.x), f.y);
}

float CloudNoise(float2 p)
{
    float sum = 0.0f;
    float amplitude = 0.5f;
    [unroll]
    for (int octave = 0; octave < 4; ++octave)
    {
        sum += ValueNoise(p) * amplitude;
        p = p * 2.03f + float2(17.1f, 9.2f);
        amplitude *= 0.5f;
    }
    return sum;
}

float AuroraCurtain(float2 sky)
{
    float time = iTime;
    float drift = time * 0.035f;
    float2 flowUV = float2(sky.x * 3.2f - drift, sky.y * 1.7f);
    float broadNoise = CloudNoise(flowUV * float2(2.0f, 1.0f) + float2(time * 0.08f, -time * 0.025f));
    float fineNoise = CloudNoise(flowUV * float2(8.0f, 3.0f) + float2(-time * 0.11f, time * 0.04f));
    float warp = (broadNoise - 0.5f) * 0.22f + (fineNoise - 0.5f) * 0.055f;

    float ribbonA = 0.24f + 0.055f * sin(sky.x * 5.0f + time * 0.16f);
    float ribbonB = 0.39f + 0.075f * sin(sky.x * 3.1f - time * 0.12f + 1.4f);
    float ribbonC = 0.53f + 0.045f * sin(sky.x * 6.4f + time * 0.09f + 2.1f);
    float widthA = exp(-pow((sky.y - ribbonA - warp) * 22.0f, 2.0f));
    float widthB = exp(-pow((sky.y - ribbonB - warp * 0.75f) * 17.0f, 2.0f));
    float widthC = exp(-pow((sky.y - ribbonC - warp * 0.5f) * 25.0f, 2.0f));

    // 微細なノイズが光を帯びた襞（ひだ）を生み出し、広大な領域がそこに息吹を与える。
    float folds = smoothstep(0.25f, 0.82f, fineNoise + broadNoise * 0.32f);
    float curtains = (widthA * 0.75f + widthB + widthC * 0.55f) * (0.22f + folds * 0.9f);
    curtains *= smoothstep(-0.08f, 0.16f, sky.y) * (1.0f - smoothstep(0.68f, 0.9f, sky.y));
    return curtains;
}

void mainImage(out float4 fragColor, in float2 fragCoord)
{
    float3 ray = SkyRay(fragCoord);
    
    // 深いネイビーのグラデーションに、柔らかな、傾いた銀河の帯が重なる。
    float horizon = saturate(ray.y * 0.5f + 0.5f);
    float3 sky = lerp(float3(0.006f, 0.009f, 0.035f), float3(0.025f, 0.045f, 0.12f), pow(horizon, 0.7f));
    float band = exp(-pow((ray.y + ray.x * 0.23f + ray.z * 0.12f) * 8.0f, 2.0f));
    float cloud = 0.5f + 0.5f * sin(ray.x * 17.0f + sin(ray.z * 13.0f)) * sin(ray.z * 21.0f + ray.y * 9.0f);
    sky += band * (0.012f + cloud * 0.018f) * float3(0.55f, 0.62f, 1.0f);

    float longitude = atan2(ray.x, ray.z);
    float latitude = asin(clamp(ray.y, -1.0f, 1.0f));
    float aurora = AuroraCurtain(float2(longitude, latitude));
    float3 auroraColor = lerp(float3(0.12f, 0.95f, 0.48f), float3(0.28f, 0.56f, 1.0f), saturate(0.5f + 0.5f * sin(longitude * 1.8f + iTime * 0.11f)));
    sky += aurora * auroraColor * 0.34f;
    sky += pow(saturate(aurora), 2.0f) * float3(0.24f, 0.72f, 0.45f) * 0.24f;

    float stars = StarLayer(ray, 150.0f, 0.965f, 0.10f) * 0.8f;
    stars += StarLayer(ray, 310.0f, 0.985f, 0.075f) * 1.15f;
    stars += StarLayer(ray, 620.0f, 0.994f, 0.06f) * 1.6f;
    float3 starColor = lerp(float3(0.60f, 0.72f, 1.0f), float3(1.0f, 0.82f, 0.62f), Hash21(floor(ray.xz * 900.0f)));
    sky += stars * starColor;
    
    // 明るくくっきりとした星にほのかに四方向に光の筋（回折）を伸ばす。
    float bright = pow(saturate(stars - 0.82f), 5.0f) * 5.0f;
    sky += bright * float3(0.72f, 0.84f, 1.0f);
    fragColor = float4(sky, 1.0f);
}
