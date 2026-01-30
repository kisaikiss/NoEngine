cbuffer gMaterial : register(b0)
{
    float maxAge;
    float pad[3];
};

struct PSInput
{
    float4 position : SV_POSITION;
    float age : TEXCOORD0;
    float4 color : COLOR;
};

float4 main(PSInput IN) : SV_TARGET
{
    float t = saturate(IN.age / maxAge);
    float alpha = (1.0f - t) * IN.color.a;
    float4 col = IN.color;
    return float4(col.rgb, alpha);
}