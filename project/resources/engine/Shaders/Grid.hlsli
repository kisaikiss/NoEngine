cbuffer GridConstants : register(b0)
{
    row_major float4x4 viewProjection;
    float4 originSpacing;
    float4 centerRadius;
    float4 widthsMajor;
    float4 options;
    float4 minorColor;
    float4 majorColor;
    float4 cameraPosition;
};

struct GridVertex
{
    float4 position : SV_POSITION;
    float3 worldPosition : TEXCOORD0;
};

float2 PlaneCoordinates(float3 p)
{
    if (options.x < 0.5) return p.xz;
    if (options.x < 1.5) return p.xy;
    return p.yz;
}
