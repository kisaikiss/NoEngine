struct VSOutput
{
    float4 pos : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

struct PSOutput
{
    float4 color : SV_TARGET0;
};

struct BlurParams
{
    float2 texelSize;
    float2 direction; // (1,0)=横 (0,1)=縦
};
ConstantBuffer<BlurParams> gBlur : register(b0);

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

static const float kWeights[5] = { 0.2270270270f, 0.1945945946f, 0.1216216216f, 0.0540540541f, 0.0162162162f };

PSOutput main(VSOutput input)
{
    PSOutput o;
    float3 result = gTexture.Sample(gSampler, input.texcoord).rgb * kWeights[0];

    [unroll]
    for (int i = 1; i < 5; ++i)
    {
        float2 offset = gBlur.direction * gBlur.texelSize * float(i) * 1.5f;
        result += gTexture.Sample(gSampler, input.texcoord + offset).rgb * kWeights[i];
        result += gTexture.Sample(gSampler, input.texcoord - offset).rgb * kWeights[i];
    }

    o.color = float4(result, 1.0f);
    return o;
}