struct Joint
{
    float4x4 PosMatrix;
    float4x4 NrmMatrix; // Inverse-transpose of PosMatrix
};
StructuredBuffer<Joint> gJoints : register(t0);

struct Vertex
{
    float4 position;
    float4 texcoord;
    float3 normal;
};
StructuredBuffer<Vertex> gInputVertices : register(t1);

struct VertexInfluence
{
    float4 jointWeights;
    uint4 jointIndices;
};
StructuredBuffer<VertexInfluence> gInfluences : register(t2);

RWStructuredBuffer<Vertex> gOutputVertices : register(u0);

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
        VertexInfluence influence = gInfluences[vertexIndex];
        
        Vertex skinned;
        skinned.texcoord = input.texcoord;
        
        float4 weights = influence.jointWeights / dot(influence.jointWeights, 1.0f);
        float4 position = input.position;
        float4 skinnedPos = mul(position, gJoints[influence.jointIndices.x].PosMatrix) * weights.x;
        skinnedPos += mul(position, gJoints[influence.jointIndices.y].PosMatrix) * weights.y;
        skinnedPos += mul(position, gJoints[influence.jointIndices.z].PosMatrix) * weights.z;
        skinnedPos += mul(position, gJoints[influence.jointIndices.w].PosMatrix) * weights.w;
        skinned.position = skinnedPos;
        
        float3 normal = input.normal;
        float3 skinnedNor = mul(normal, (float3x3) gJoints[influence.jointIndices.x].NrmMatrix) * weights.x;
        skinnedNor += mul(normal, (float3x3) gJoints[influence.jointIndices.y].NrmMatrix) * weights.y;
        skinnedNor += mul(normal, (float3x3) gJoints[influence.jointIndices.z].NrmMatrix) * weights.z;
        skinnedNor += mul(normal, (float3x3) gJoints[influence.jointIndices.w].NrmMatrix) * weights.w;
        skinned.normal = skinnedNor;
        
        gOutputVertices[vertexIndex] = skinned;
    }
}