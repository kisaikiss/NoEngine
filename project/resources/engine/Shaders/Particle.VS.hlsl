struct WorldMatrix
{
    float4x4 world;
    float4 color;
};
StructuredBuffer<WorldMatrix> gWorldMatrices : register(t1);

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
};

struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float4 color : COLOR0;
};

struct CameraMatrix
{
    float4x4 viewProjection;
    float3 worldPosition;
    float fov;
};
ConstantBuffer<CameraMatrix> gCameraMatrix : register(b2);


struct BaseIndex
{
    int index;
};
ConstantBuffer<BaseIndex> gBaseIndex : register(b3);

VertexShaderOutput main(VertexShaderInput input, uint instanceId : SV_InstanceID)
{
    VertexShaderOutput output;
   
    
    float4 position = input.position;
    float4x4 worldMatrix = gWorldMatrices[instanceId + gBaseIndex.index].world;
    
  
    float4x4 viewProjection = gCameraMatrix.viewProjection;
    float4x4 wvp = mul(worldMatrix, viewProjection);
    
    output.position = mul(position, wvp);
    output.texcoord = input.texcoord;
    output.color = gWorldMatrices[instanceId + gBaseIndex.index].color;
    return output;
}