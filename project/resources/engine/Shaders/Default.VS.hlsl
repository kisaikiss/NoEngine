#include "Default.hlsli"

struct WorldMatrix
{
    float4x4 world;
};
ConstantBuffer<WorldMatrix> gWorldMatrix : register(b1);

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    float4 worldPos = mul(input.position, gWorldMatrix.world);
    output.position = mul(worldPos, gCameraMatrix.viewProjection);
    output.texcoord = input.texcoord;
    return output;
}