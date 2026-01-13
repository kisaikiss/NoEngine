
struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

struct CameraMatrix
{
    float4x4 viewProjection;
};
ConstantBuffer<CameraMatrix> gCameraMatrix : register(b2);