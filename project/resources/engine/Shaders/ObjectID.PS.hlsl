#include "Default.hlsli"

struct PSOutput
{
    float4 id : SV_Target0;
};


struct ObjectID
{
    float4 id;
};
ConstantBuffer<ObjectID> gObjectID : register(b0);

struct PickMaterial
{
    float4x4 uvTransform;
    float opacity;
};
ConstantBuffer<PickMaterial> gPickMaterial : register(b3);
Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

PSOutput main(VertexShaderOutput input)
{
    float2 uv = mul(float4(input.texcoord, 0.0f, 1.0f), gPickMaterial.uvTransform).xy;
    if (gPickMaterial.opacity * gTexture.Sample(gSampler, uv).a <= 0.0f)
    {
        discard;
    }

    PSOutput output;
    output.id = gObjectID.id;
    return output;
}
