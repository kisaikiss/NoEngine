struct WorldMatrix
{
    float4x4 world;
    float4x4 worldInversT;
};
struct CameraMatrix
{
    float4x4 viewProjection;
};
ConstantBuffer<WorldMatrix> gWorldMatrix : register(b1);
ConstantBuffer<CameraMatrix> gCameraMatrix : register(b2);

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    
    float4 position = input.position;
    float4 worldPos = mul(position, gWorldMatrix.world);
    
    output.position = mul(worldPos, gCameraMatrix.viewProjection);
    output.texcoord = input.texcoord;
    output.normal = mul(input.normal, (float3x3)gWorldMatrix.worldInversT).xyz;
    
    return output;
}