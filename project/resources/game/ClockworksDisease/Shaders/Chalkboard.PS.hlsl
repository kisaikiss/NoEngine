#include "BackgroundCommon.hlsli"

float BoardHash(float2 p)
{
    return frac(sin(dot(p, float2(127.1f, 311.7f))) * 43758.5453f);
}

float BoardNoise(float2 p)
{
    float2 cell = floor(p);
    float2 f = frac(p);
    f = f * f * (3.0f - 2.0f * f);
    return lerp(lerp(BoardHash(cell), BoardHash(cell + float2(1, 0)), f.x),
        lerp(BoardHash(cell + float2(0, 1)), BoardHash(cell + float2(1, 1)), f.x), f.y);
}

void mainImage(out float4 fragColor, in float2 fragCoord)
{
    float2 uv = fragCoord / iResolution;
    float2 p = fragCoord / iResolution.y;
    // 時間に依存しない色むら・細粒・横長の消し跡を重ねる。
    float clouds = BoardNoise(p * 5.0f);
    float grain = BoardHash(floor(fragCoord));
    float erased = BoardNoise(p * float2(3.0f, 28.0f)) * BoardNoise(p * 9.0f + 17.0f);
    float scratches = pow(BoardNoise(p * float2(2.0f, 210.0f)), 10.0f);
    float vignette = saturate(1.0f - 0.35f * dot(uv - 0.5f, uv - 0.5f));
    // param: x=色むら、y=粒、z=消し跡、w=擦り傷の強さ。
    float shade = 0.8f + (clouds - 0.5f) * iParam.x + (grain - 0.5f) * iParam.y;
    float3 board = float3(0.018f, 0.045f, 0.035f) * shade;
    board += float3(0.025f, 0.033f, 0.027f) * (erased * iParam.z + scratches * iParam.w);
    fragColor = float4(max(board * vignette, 0.0f), 1.0f);
}
