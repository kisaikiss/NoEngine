#include "BackgroundCommon.hlsli"

void mainImage(out float4 fragColor, in float2 fragCoord)
{
    float2 uv = fragCoord / iResolution.xy;
    fragColor = float4(0.5f + 0.5f * cos(iTime + uv.xyx + float3(0, 2, 4)), 1.0f);
}