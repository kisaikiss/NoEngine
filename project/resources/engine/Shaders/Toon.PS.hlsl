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
    output.color = 0;

    float4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    
    for (int i = 0; i < gLightNums.directionalLightNum; i++)
    {
        float NdotL = dot(normalize(input.normal), -gDirectionalLights[i].direction);
        float toonIntensity = (NdotL > 0.6) ? 1.0 : 0.3;
        float totalIntensity = toonIntensity * gDirectionalLights[i].intensity;
        output.color += gMaterial.color * textureColor * gDirectionalLights[i].color * totalIntensity;
    }
        
    output.color.a = gMaterial.color.a;
    return output;
}