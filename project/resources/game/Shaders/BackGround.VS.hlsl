
cbuffer gWorld : register(b0)
{
    float4x4 WorldMat;
    float4x4 UVTransform;
    float4x4 ViewProjMat;
    float time;
};

struct VSInput
{
    float3 position : POSITION;
    float2 uv : TEXCOORD0;
};

struct VSOutput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
};

VSOutput main(VSInput input)
{
    VSOutput output;

    output.worldPos = mul(float4(input.position, 1.0f), WorldMat).xyz;
    output.position = mul(float4(output.worldPos, 1.0f), ViewProjMat);
    output.uv = mul(float4(input.uv.x, input.uv.y, 0.0f, 1.0f), UVTransform).xy;
    
    return output;
}