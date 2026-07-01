struct Joint
{
    float4x4 PosMatrix;
    float4x4 NrmMatrix; // Inverse-transpose of PosMatrix
};
StructuredBuffer<Joint> gJoints : register(t0);

struct Vertex
{
    float3 position;
    float2 texcoord;
    float3 normal;
    float4 jointWeights;
    uint4 jointIndices;
};
StructuredBuffer<Vertex> gInputVertices : register(t1);

struct OutVertex
{
    float3 position;
    float2 texcoord;
    float3 normal;
};
RWStructuredBuffer<OutVertex> gOutputVertices : register(u0);

struct SkinningInformation
{
    uint numVertices;
};
ConstantBuffer<SkinningInformation> gSkinningInformation : register(b0);


[numthreads(1024, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint vertexIndex = DTid.x;
    if (vertexIndex < gSkinningInformation.numVertices)
    {
        Vertex input = gInputVertices[vertexIndex];
        OutVertex skinned;
        skinned.texcoord = input.texcoord;
        
        float4 weights = input.jointWeights / dot(input.jointWeights, 1.0f);
        float4 position = float4(input.position, 1.0f);
        float4 skinnedPos = mul(position, gJoints[input.jointIndices.x].PosMatrix) * weights.x;
        skinnedPos += mul(position, gJoints[input.jointIndices.y].PosMatrix) * weights.y;
        skinnedPos += mul(position, gJoints[input.jointIndices.z].PosMatrix) * weights.z;
        skinnedPos += mul(position, gJoints[input.jointIndices.w].PosMatrix) * weights.w;
        skinned.position = float3(skinnedPos.x / skinnedPos.w, skinnedPos.y / skinnedPos.w, skinnedPos.z / skinnedPos.w);
        
        float3 normal = input.normal;
        float3 skinnedNor = mul(normal, (float3x3) gJoints[input.jointIndices.x].NrmMatrix) * weights.x;
        skinnedNor += mul(normal, (float3x3) gJoints[input.jointIndices.y].NrmMatrix) * weights.y;
        skinnedNor += mul(normal, (float3x3) gJoints[input.jointIndices.z].NrmMatrix) * weights.z;
        skinnedNor += mul(normal, (float3x3) gJoints[input.jointIndices.w].NrmMatrix) * weights.w;
        skinned.normal = skinnedNor;
        
        gOutputVertices[vertexIndex] = skinned;
    }
}