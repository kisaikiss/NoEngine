struct Material
{
    float4 color;
    float shininess;
    float environmentCoefficient;
    float unlit;
    float receiveShadow;
    float4x4 uvTransform;
};
ConstantBuffer<Material> gMaterial : register(b0);
Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

float4 SampleBaseColor(float2 uv)
{
    float2 transformedUV = mul(float4(uv, 0.0f, 1.0f), gMaterial.uvTransform).xy;
    return gMaterial.color * gTexture.Sample(gSampler, transformedUV);
}
