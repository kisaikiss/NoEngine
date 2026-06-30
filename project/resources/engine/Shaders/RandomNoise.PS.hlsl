#include "Random.hlsli"

struct VSOutput
{
    float4 pos : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

struct PSOutput
{
    float4 color : SV_TARGET0;
};

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

cbuffer SeedCB : register(b0)
{
    float time;
    float3 _pad;
};

PSOutput main(VSOutput input)
{
    PSOutput o;
    float random = rand2dTo1d(input.texcoord * time);
    
    o.color = gTexture.Sample(gSampler, input.texcoord) * float4(random, random, random, 1.0f);
    
    return o;
}