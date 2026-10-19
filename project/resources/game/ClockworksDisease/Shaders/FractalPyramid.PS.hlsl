#include "BackgroundCommon.hlsli"

float3 palette(float d)
{
    return lerp(float3(0.2f, 0.7f, 0.9f), float3(1.0f, 0.0f, 1.0f), d);
}

// 行列を使わず展開
float2 rot2(float2 p, float a)
{
    float c = cos(a);
    float s = sin(a);
    return float2(p.x * c + p.y * s, -p.x * s + p.y * c);
}

float Map(float3 p)
{
    for (int i = 0; i < 8; ++i)
    {
        float t = iTime * 0.2f;
        p.xz = rot2(p.xz, t);
        p.xy = rot2(p.xy, t * 1.89f);
        p.xz = abs(p.xz);
        p.xz -= 0.5f;
    }
    return dot(sign(p), p) / 5.0f;
}

float4 RayMarch(float3 ro, float3 rd)
{
    float t = 0.0f;
    float3 col = float3(0.0f, 0.0f, 0.0f);
    float d = 0.0f;

    [loop]
    for (int i = 0; i < 64; i++)
    {
        float3 p = ro + rd * t;
        d = Map(p) * 0.5f;
        if (d < 0.02f)
        {
            break;
        }
        if (d > 100.0f)
        {
            break;
        }
        col += palette(length(p) * 0.1f) / (400.0f * d);
        t += d;
    }
    return float4(col, 1.0f / (d * 100.0f));
}

void mainImage(out float4 fragColor, in float2 fragCoord)
{
    float2 uv = (fragCoord - (iResolution.xy / 2.0f)) / iResolution.x;
    float3 ro = float3(0.0f, 0.0f, -50.0f);
    ro.xz = rot2(ro.xz, iTime);
    float3 cf = normalize(-ro);
    float3 cs = normalize(cross(cf, float3(0.0f, 1.0f, 0.0f)));
    float3 cu = normalize(cross(cf, cs));

    float3 uuv = ro + cf * 3.0f + uv.x * cs + uv.y * cu;

    float3 rd = normalize(uuv - ro);

    fragColor = RayMarch(ro, rd);
}