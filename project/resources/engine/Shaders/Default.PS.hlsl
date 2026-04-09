#include "Default.hlsli"

struct Material
{
    float4 color;
    float shininess;
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

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color = 0;

    float4 textureColor = gTexture.Sample(gSampler, input.texcoord);
    
    float3 toEye = normalize(gCameraMatrix.worldPosition - input.worldPosition);
    
    for (int i = 0; i < gLightNums.directionalLightNum; i++)
    {
        // 拡散反射
        float NdotL = dot(normalize(input.normal), -gDirectionalLights[i].direction);
        float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
        float4 diffuse = gDirectionalLights[i].color * cos * gDirectionalLights[i].intensity;
        
        // 鏡面反射
        float3 halfVector = normalize(-gDirectionalLights[i].direction + toEye);
        float NdotH = dot(normalize(input.normal), halfVector);
        float specularPow = pow(saturate(NdotH), gMaterial.shininess);
        float3 specular = gDirectionalLights[i].color.rgb * gDirectionalLights[i].intensity * specularPow * float3(1.f, 1.f, 1.f);
        
         //拡散反射*鏡面反射
        float4 color = 0;
        color.rgb = diffuse.rgb + specular;
        
        output.color += gMaterial.color * textureColor * color;
    }
    
        
        output.color.a = gMaterial.color.a * textureColor.a;
    return output;
}