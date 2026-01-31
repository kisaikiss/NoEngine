
struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

struct CameraMatrix
{
    float4x4 viewProjection;
    float3 worldPosition;
    float fov;
};
ConstantBuffer<CameraMatrix> gCameraMatrix : register(b2);