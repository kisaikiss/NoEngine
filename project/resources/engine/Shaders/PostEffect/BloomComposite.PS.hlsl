struct VSOutput
{
    float4 pos : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

struct PSOutput
{
    float4 color : SV_TARGET0;
};

struct BloomComposite
{
    float intensity;
    float3 pad;
};
ConstantBuffer<BloomComposite> gComposite : register(b0);

Texture2D<float4> gSceneColor : register(t0);
Texture2D<float4> gBloomColor : register(t1);
SamplerState gSampler : register(s0);

PSOutput main(VSOutput input)
{
    PSOutput o;
    float3 scene = gSceneColor.Sample(gSampler, input.texcoord).rgb;
    float3 bloom = gBloomColor.Sample(gSampler, input.texcoord).rgb;
    o.color = float4(scene + bloom * gComposite.intensity, 1.0f);
    return o;
}