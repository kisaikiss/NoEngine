struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};
Texture2D<float4> gTexture : register(t0);
Texture2D<float4> gMask : register(t1);
SamplerState gSampler : register(s0);

cbuffer gMaterial : register(b0)
{
    float4 color;
};

cbuffer gMaskParams : register(b1)
{
    float fill; // 0..1 （UV.x に対する切り詰め量）
    int useMask; // 0 = UVクリップを使う, 1 = テクスチャマスクを使う
    float2 padding; // パディング
}

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    float4 tex = gTexture.Sample(gSampler, input.texcoord);

    float alpha = tex.a;

    float mask = 0;
    if(useMask == 1)
    {
       mask = gMask.Sample(gSampler, input.texcoord).r;
        float maskAllow = 1.0 - mask;

        float fillAllow = step(input.texcoord.x, fill);

        alpha *= maskAllow * fillAllow;
    }

    if(alpha < 0.5)
    {
        discard;
    }
    
    output.color = float4(tex.rgb * color.rgb, alpha * color.a);
    return output;
}
