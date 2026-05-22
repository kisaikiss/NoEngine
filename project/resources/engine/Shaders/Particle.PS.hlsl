struct Material
{
    float4 color;
};
ConstantBuffer<Material> gMaterial : register(b0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};
Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float4 color : COLOR0;
};


PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color = 0;
    
    float4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    output.color = input.color * textureColor;
    if (output.color.a == 0.0)
    {
        discard;
    }
    
    return output;
}