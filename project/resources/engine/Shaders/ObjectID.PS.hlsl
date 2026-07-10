#include "Default.hlsli"

struct PSOutput
{
    float4 id : SV_Target0;
};


struct ObjectID
{
    float4 id;
};
ConstantBuffer<ObjectID> gObjectID : register(b0);

PSOutput main(VertexShaderOutput input)
{
    PSOutput output;
    output.id = gObjectID.id;
    return output;
}