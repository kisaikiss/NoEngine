
struct WorldMatrix
{
    float4x4 world;
};
ConstantBuffer<WorldMatrix> gWorldMatrix : register(b0);

struct CameraMatrix
{
    float4x4 viewProjection;
    float3 worldPosition;
    float fov;
};
ConstantBuffer<CameraMatrix> gCameraMatrix : register(b1);

struct VertexShaderInput
{
    float3 position : POSITION0;
};

struct VertexShaderOutput
{
    float4 position : SV_Position;
    float3 texcoord : TEXCOORD0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    float4 worldPos = mul(float4(input.position, 1.f), gWorldMatrix.world);
    output.position = mul(worldPos, gCameraMatrix.viewProjection);
    output.texcoord = input.position;
    
    return output;
}