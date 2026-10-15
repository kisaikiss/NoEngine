struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

// アンチエイリアス付きフォントマスク。FontManagerでDXGI_FORMAT_R8_UNORMとしてベイクしているため単一チャンネル(.r)。
Texture2D<float> gTexture : register(t0);
SamplerState gSampler : register(s0);

cbuffer gMaterial : register(b0)
{
    float4 color;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    float alpha = gTexture.Sample(gSampler, input.texcoord);

    if (alpha <= 0.001f)
    {
        discard;
    }

    output.color = float4(color.rgb, color.a * alpha);
    return output;
}
