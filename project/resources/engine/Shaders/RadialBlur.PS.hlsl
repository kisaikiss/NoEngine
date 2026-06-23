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

PSOutput main(VSOutput input)
{
    PSOutput o;
    const float2 kCenter = float2(0.5f, 0.5f);
    const int kNumSamples = 10;
    const float kBlurWidth = 0.01f;
    
    float2 direction = input.texcoord - kCenter;
    float3 outputColor = float3(0.0f, 0.0f, 0.0f);
    for (int sampleIndex = 0; sampleIndex < kNumSamples; ++sampleIndex)
    {
        float2 texcoord = input.texcoord + direction * kBlurWidth * float(sampleIndex);
        outputColor.rgb += gTexture.Sample(gSampler, texcoord).rgb;
    }
    outputColor.rgb *= rcp(float(kNumSamples));
    
    o.color.rgb = outputColor;
    o.color.a = 1.0f;
    
    return o;
}