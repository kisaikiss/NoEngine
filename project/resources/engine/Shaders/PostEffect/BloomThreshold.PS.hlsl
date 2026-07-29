struct VSOutput
{
    float4 pos : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

struct PSOutput
{
    float4 color : SV_TARGET0;
};

struct BloomThreshold
{
    float threshold;
    float softKnee;
    float2 pad;
};
ConstantBuffer<BloomThreshold> gThreshold : register(b0);

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

float Luminance(float3 c)
{
    return dot(c, float3(0.2126f, 0.7152f, 0.0722f));
}

PSOutput main(VSOutput input)
{
    PSOutput o;
    float3 color = gTexture.Sample(gSampler, input.texcoord).rgb;
    float lum = Luminance(color);

    float knee = gThreshold.threshold * gThreshold.softKnee;
    float soft = clamp(lum - gThreshold.threshold + knee, 0.0f, 2.0f * knee);
    soft = soft * soft / (4.0f * knee + 1e-5f);

    float contribution = max(soft, lum - gThreshold.threshold) / max(lum, 1e-5f);
    o.color = float4(color * contribution, 1.0f);
    return o;
}