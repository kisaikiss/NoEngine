#include "Particle.hlsli"

StructuredBuffer<Particle> gParticles : register(t1);

struct PerView
{
    float4x4 viewProjection;
    float4x4 billboardMatrix;
};
ConstantBuffer<PerView> gParView : register(b2);

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
};

struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
    float4 color : COLOR0;
};

VertexShaderOutput main(VertexShaderInput input, uint instanceId : SV_InstanceID)
{
    VertexShaderOutput output;
   
    Particle particle = gParticles[instanceId];
    float4x4 worldMatrix = gParView.billboardMatrix;
    worldMatrix[0] *= particle.scale.x;
    worldMatrix[1] *= particle.scale.y;
    worldMatrix[2] *= particle.scale.z;
    worldMatrix[3].xyz = particle.translate;
    output.position = mul(input.position, mul(worldMatrix, gParView.viewProjection));
    output.texcoord = input.texcoord;
    output.color = particle.color;
    
    return output;
}