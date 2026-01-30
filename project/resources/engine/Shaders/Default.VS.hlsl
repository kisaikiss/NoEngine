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
    float3 normal : NORMAL0;
#ifdef ENABLE_SKINNING
    float4 jointWeights : WEIGHT0;
    uint4 jointIndices : INDEX0;
#endif
};

#ifdef ENABLE_SKINNING
struct Joint
{
    float4x4 PosMatrix;
    float4x4 NrmMatrix; // Inverse-transpose of PosMatrix
};
StructuredBuffer<Joint> gJoints : register(t1);
#endif

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
   
    
    float4 position = input.position;
    float3 normal = input.normal;
#ifdef ENABLE_SKINNING
    float4 weights = input.jointWeights / dot(input.jointWeights, 1.0f);
    
    float4 skinnedPos = mul(position, gJoints[input.jointIndices.x].PosMatrix) * weights.x;
    skinnedPos += mul(position, gJoints[input.jointIndices.y].PosMatrix) * weights.y;
    skinnedPos += mul(position, gJoints[input.jointIndices.z].PosMatrix) * weights.z;
    skinnedPos += mul(position, gJoints[input.jointIndices.w].PosMatrix) * weights.w;
    position = skinnedPos;
    
    float3 skinnedNor = mul(normal, (float3x3) gJoints[input.jointIndices.x].NrmMatrix) * weights.x;
    skinnedNor += mul(normal, (float3x3) gJoints[input.jointIndices.y].NrmMatrix) * weights.y;
    skinnedNor += mul(normal, (float3x3) gJoints[input.jointIndices.z].NrmMatrix) * weights.z;
    skinnedNor += mul(normal, (float3x3) gJoints[input.jointIndices.w].NrmMatrix) * weights.w;
    normal = skinnedNor;
#endif
    
    float4 worldPos = mul(position, gWorldMatrix.world);
    
    output.position = mul(worldPos, gCameraMatrix.viewProjection);
    output.texcoord = input.texcoord;
    output.normal = normalize(mul(normal, (float3x3) gWorldMatrix.world));
    return output;
}