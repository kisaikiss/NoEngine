#include "Default.hlsli"

struct PSOutput
{
    float4 worldPos : SV_Target0;
    float4 normal : SV_Target1;
    float4 id : SV_Target2;
};

struct ObjectID
{
    float4 id;
};
ConstantBuffer<ObjectID> gObjectID : register(b0);

PSOutput main(VertexShaderOutput input)
{
    PSOutput output;
    output.worldPos = float4(input.worldPosition, 1.0f);
     // Normal（正規化して格納）
    float3 n = normalize(input.normal);
    output.normal = float4(n * 0.5 + 0.5, 1.0); // [-1,1] → [0,1]
    output.id = gObjectID.id;
    return output;
}