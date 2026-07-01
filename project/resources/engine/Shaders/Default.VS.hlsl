#include "Default.hlsli"
struct WorldMatrix
{
    float4x4 world;
    float4x4 worldIT;
};
ConstantBuffer<WorldMatrix> gWorldMatrix : register(b1);

struct VertexShaderInput
{
    float3 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
   
    
    float4 position = float4(input.position, 1.0f);
    float3 normal = input.normal;
    
    float4 worldPos = mul(position, gWorldMatrix.world);
    
    output.position = mul(worldPos, gCameraMatrix.viewProjection);
    output.texcoord = input.texcoord;
    output.normal = normalize(mul(normal, (float3x3) gWorldMatrix.worldIT));
    output.worldPosition = mul(float4(input.position, 1.0f), gWorldMatrix.world).xyz;
    return output;
}