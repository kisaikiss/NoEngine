struct VSOutput
{
    float4 pos : SV_POSITION;
    float2 tedcoord : TEXCOORD0;
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
    o.color = gTexture.Sample(gSampler, input.tedcoord);
    
    // 周囲を0に、中心になるほど明るくなるように計算で調整
    float2 correct = input.tedcoord * (1.0f - input.tedcoord.yx);
    // correctだけで計算すると中心の最大値が0.0625で暗すぎるのでScaleで調整
    float vignette = correct.x * correct.y * 16.0f;
    
    vignette = saturate(pow(vignette, 0.8f));
    
    o.color.rgb *= vignette;
    return o;
}