#include "BackgroundCommon.hlsli"

static const float PI = 3.14159265f;

float hash21(float2 p)
{
    p = frac(p * float2(123.34f, 456.21f));
    p += dot(p, p + 45.32f);
    return frac(p.x * p.y);
}

float noise2(float2 p)
{
    float2 cell = floor(p);
    float2 f = frac(p);
    f = f * f * (3.0f - 2.0f * f);
    float a = hash21(cell);
    float b = hash21(cell + float2(1.0f, 0.0f));
    float c = hash21(cell + float2(0.0f, 1.0f));
    float d = hash21(cell + float2(1.0f, 1.0f));
    return lerp(lerp(a, b, f.x), lerp(c, d, f.x), f.y);
}

float fbm(float2 p)
{
    float value = 0.0f;
    float amplitude = 0.5f;
    [unroll]
    for (int octave = 0; octave < 5; ++octave)
    {
        value += noise2(p) * amplitude;
        p = p * 2.03f + 17.1f;
        amplitude *= 0.5f;
    }
    return value;
}

void mainImage(out float4 fragColor, in float2 fragCoord)
{
    float2 uv = fragCoord / iResolution;
    float2 screen = uv * 2.0f - 1.0f;
    float tanHalfFov = tan(iCameraParams.x * 0.5f);
    float3 ray = normalize(iCameraForward.xyz
        + iCameraRight.xyz * (screen.x * tanHalfFov * iCameraParams.y)
        + iCameraUp.xyz * (screen.y * tanHalfFov));

    // 原点中心の空球との交点を使い、カメラ移動でも空模様に視差を出す。
    float radius = max(iParam.x, 100.0f);
    float3 origin = iCameraPosition.xyz;
    float b = dot(origin, ray);
    float c = dot(origin, origin) - radius * radius;
    float t = max(0.0f, -b + sqrt(max(b * b - c, 0.0f)));
    float3 skyPoint = origin + ray * t;
    float3 skyDirection = normalize(skyPoint);

    float longitude = atan2(skyDirection.z, skyDirection.x);
    float latitude = asin(clamp(skyDirection.y, -1.0f, 1.0f));
    float2 skyUV = float2(longitude / (2.0f * PI) + 0.5f, latitude / PI + 0.5f);
    float cloudScale = (iParam.y > 0.0f) ? iParam.y : 7.0f;
    float drift = iTime * 0.008f;
    float cloud = fbm(float2(skyUV.x * cloudScale + drift, skyUV.y * cloudScale));
    cloud = smoothstep(0.48f, 0.72f, cloud);

    float horizon = smoothstep(-0.18f, 0.65f, skyDirection.y);
    float3 horizonColor = float3(0.88f, 0.55f, 0.34f);
    float3 zenithColor = float3(0.08f, 0.25f, 0.58f);
    float3 sky = lerp(horizonColor, zenithColor, horizon);
    float3 cloudColor = lerp(float3(0.72f, 0.78f, 0.88f), float3(1.0f, 0.88f, 0.72f), horizon);
    sky = lerp(sky, cloudColor, cloud * (0.52f + 0.30f * horizon));

    // 太陽の位置はiParam.zwの経度・緯度で調整可能。
    float sunLongitude = iParam.z;
    float sunLatitude = iParam.w;
    float3 sunDirection = float3(cos(sunLatitude) * cos(sunLongitude), sin(sunLatitude), cos(sunLatitude) * sin(sunLongitude));
    float sun = pow(saturate(dot(skyDirection, sunDirection)), 320.0f);
    float glow = pow(saturate(dot(skyDirection, sunDirection)), 12.0f);
    sky += float3(1.0f, 0.72f, 0.42f) * (sun + glow * 0.24f);

    fragColor = float4(sky, 1.0f);
}
