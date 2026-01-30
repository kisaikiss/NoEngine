#include "Default.hlsli"

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

struct LightNums
{
    int directionalLightNum;
    int pointLightNum;
    int spotLightNum;
};
ConstantBuffer<LightNums> gLightNums : register(b3);

struct DirectionalLight
{
    float4 color;
    float3 direction;
    float intensity;
};
StructuredBuffer<DirectionalLight> gDirectionalLights : register(t2);

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    
    for (int i = 0; i < gLightNums.directionalLightNum; i++)
    {
        float NdotL = dot(normalize(input.normal), -gDirectionalLights[i].direction);
        float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
        output.color += gMaterial.color * textureColor * gDirectionalLights[i].color * cos * gDirectionalLights[i].intensity;
    }
        
    output.color.a = gMaterial.color.a;
    return output;
}