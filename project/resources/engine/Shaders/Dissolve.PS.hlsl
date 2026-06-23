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
Texture2D<float> gMaskTexture : register(t1);
SamplerState gSampler : register(s0);

struct Threshold
{
    float x;
    float3 pad;
};
ConstantBuffer<Threshold> gThreshold : register(b0);

PSOutput main(VSOutput input)
{
    PSOutput o;
    float mask = gMaskTexture.Sample(gSampler, input.texcoord);
    
    if (mask <= gThreshold.x)
    {
        o.color = float4(0.f, 0.f, 0.f, 0.f);
        return o;
    }
    
    float edge = 1.0f - smoothstep(gThreshold.x, gThreshold.x + 0.03f, mask);
    o.color = gTexture.Sample(gSampler, input.texcoord);
    
    o.color.rgb += edge * float3(1.0f, 0.4f, 0.3f);
    
	return o;
}