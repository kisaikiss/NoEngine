#include "Default.hlsli"

struct PSOutput
{
    float4 worldPos : SV_Target0; // GBuffer0
};

PSOutput main(VertexShaderOutput input)
{
    PSOutput output;
    output.worldPos = float4(input.worldPosition, 1.0f);
    return output;
}