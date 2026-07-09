#include "Default.hlsli"

struct Material
{
    float4 color;
    float shininess;
    float environmentCoefficient;
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

struct PointLight
{
    float4 color;
    float3 position;
    float intensity;
    float radius;
    float decay;
};
StructuredBuffer<PointLight> gPointLights : register(t3);

struct SpotLight
{
    float4 color;
    float3 position;
    float intensity;
    float3 direction;
    float distance;
    float decay;
    float cosAngle;
    float cosFalloffStart;
};
StructuredBuffer<SpotLight> gSpotLights : register(t4);


Texture2D<float> gShadowMask : register(t5);
TextureCube<float4> gEnvironmentTexture : register(t6);

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color = 0;

    float4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    
    int2 screenPos = int2(input.position.xy);
    float shadowFactor = gShadowMask.Load(int3(screenPos, 0));
    
    for (int i = 0; i < gLightNums.directionalLightNum; i++)
    {
        float NdotL = dot(normalize(input.normal), -gDirectionalLights[i].direction);
        float toonIntensity = (NdotL > 0.6) ? 1.0 : 0.3;
        float totalIntensity = toonIntensity * gDirectionalLights[i].intensity;
        output.color += gMaterial.color * textureColor * gDirectionalLights[i].color * totalIntensity * shadowFactor;
    }
    float3 cameraToPosition = normalize(input.worldPosition - gCameraMatrix.worldPosition);
    float3 reflectedVector = reflect(cameraToPosition, normalize(input.normal));
    float4 environmentColor = gEnvironmentTexture.Sample(gSampler, reflectedVector);
    output.color.rgb += environmentColor.rgb * gMaterial.environmentCoefficient;
    output.color.a = gMaterial.color.a;
    return output;
}