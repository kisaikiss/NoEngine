#include "Default.hlsli"

struct Material
{
    float4 color;
    float shininess;
    float environmentCoefficient;
    float2 padding;
    float4x4 uvTransform;
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

// register(t5) を配列に変更
Texture2DArray<float> gShadowMask : register(t5);
TextureCube<float4> gEnvironmentTexture : register(t6);

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color = 0;

    float4 transformedUV = mul(float4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    float4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    float3 toEye = normalize(gCameraMatrix.worldPosition - input.worldPosition);

    float4 lightColor = 0;
    int2 screenPos = int2(input.position.xy);

    // スライスのオフセットは RayGen_Shadow と同じ順序（Directional→Point→Spot）
    uint shadowSlice = 0;

    // 方向ライトの計算
    for (int i = 0; i < gLightNums.directionalLightNum; i++)
    {
        float shadowFactor = gShadowMask.Load(int4(screenPos, shadowSlice, 0));
        shadowSlice++;

        float NdotL = dot(normalize(input.normal), -gDirectionalLights[i].direction);
        float cos = pow(NdotL * 0.5f + 0.5f, 2.0f);
        float4 diffuse = gDirectionalLights[i].color * cos * gDirectionalLights[i].intensity;

        float3 halfVector = normalize(-gDirectionalLights[i].direction + toEye);
        float NdotH = dot(normalize(input.normal), halfVector);
        float specularPow = pow(saturate(NdotH), gMaterial.shininess);
        float3 specular = gDirectionalLights[i].color.rgb * gDirectionalLights[i].intensity * specularPow;

        float4 color = 0;
        color.rgb = (diffuse.rgb + specular) * shadowFactor;
        lightColor += color;
    }

    // ポイントライトの計算
    for (int j = 0; j < gLightNums.pointLightNum; j++)
    {
        float shadowFactor = gShadowMask.Load(int4(screenPos, shadowSlice, 0));
        shadowSlice++;

        float3 pointLightDirection = normalize(input.worldPosition - gPointLights[j].position);
        float distance = length(gPointLights[j].position - input.worldPosition);
        float factor = pow(saturate(-distance / gPointLights[j].radius + 1.0), gPointLights[j].decay);

        float3 pointHalfVector = normalize(-pointLightDirection + toEye);
        float pointNdotH = dot(normalize(input.normal), pointHalfVector);
        float pointCos = pow(pointNdotH * 0.5f + 0.5f, 2.0f);
        float4 pointDiffuse = gPointLights[j].color * pointCos * gPointLights[j].intensity * factor;

        float3 pointReflectLight = reflect(pointLightDirection, normalize(input.normal));
        float pointRdotE = dot(pointReflectLight, toEye);
        float pointSpecularPow = pow(saturate(pointRdotE), gMaterial.shininess) * factor;
        float3 pointSpecular = gPointLights[j].color.rgb * gPointLights[j].intensity * pointSpecularPow;

        float4 pointColor = 0;
        pointColor.rgb = (pointDiffuse.rgb + pointSpecular) * shadowFactor; // ★shadowFactorを追加
        lightColor += pointColor;
    }

    // スポットライトの計算
    for (int k = 0; k < gLightNums.spotLightNum; k++)
    {
        float shadowFactor = gShadowMask.Load(int4(screenPos, shadowSlice, 0));
        shadowSlice++;

        float3 spotLightDirectionOnSurface = normalize(input.worldPosition - gSpotLights[k].position);
        float cosAngle = dot(spotLightDirectionOnSurface, gSpotLights[k].direction);
        float falloffFactor = saturate((cosAngle - gSpotLights[k].cosAngle) / (gSpotLights[k].cosFalloffStart - gSpotLights[k].cosAngle));
        float distance = length(gSpotLights[k].position - input.worldPosition);
        float attenuationFactor = pow(saturate(-distance / gSpotLights[k].distance + 1.0), gSpotLights[k].decay);

        float3 halfVector = normalize(-spotLightDirectionOnSurface + toEye);
        float NdotH = dot(normalize(input.normal), halfVector);
        float cos = pow(NdotH * 0.5f + 0.5f, 2.0f);
        float4 diffuse = gSpotLights[k].color * cos * gSpotLights[k].intensity;

        float3 reflectLight = reflect(spotLightDirectionOnSurface, normalize(input.normal));
        float RdotE = dot(reflectLight, toEye);
        float specularPow = pow(saturate(RdotE), gMaterial.shininess);
        float3 specular = gSpotLights[k].color.rgb * gSpotLights[k].intensity * specularPow;

        float4 color = 0;
        color.rgb = (diffuse.rgb + specular) * falloffFactor * attenuationFactor * shadowFactor; // ★shadowFactorを追加
        lightColor += color;
    }

    float3 cameraToPosition = normalize(input.worldPosition - gCameraMatrix.worldPosition);
    float3 reflectedVector = reflect(cameraToPosition, normalize(input.normal));
    float4 environmentColor = gEnvironmentTexture.Sample(gSampler, reflectedVector);

    output.color += gMaterial.color * textureColor * lightColor;
    output.color.rgb += environmentColor.rgb * gMaterial.environmentCoefficient;
    output.color.a = gMaterial.color.a * textureColor.a;
    return output;
}