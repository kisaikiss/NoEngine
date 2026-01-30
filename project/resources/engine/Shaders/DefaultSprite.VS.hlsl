
struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
};


struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};


struct CameraMatrix
{
    float4x4 viewProjection;
};
ConstantBuffer<CameraMatrix> gCameraMatrix : register(b2);

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    output.position = mul(input.position, gCameraMatrix.viewProjection);
    output.texcoord = input.texcoord;
    return output;
}