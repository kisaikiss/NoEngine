struct VertexShaderOutput
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

struct PSOutput
{
    float4 id : SV_TARGET0;
};

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct ObjectID
{
    float4 id;
};
ConstantBuffer<ObjectID> gObjectID : register(b0);

PSOutput main(VertexShaderOutput input)
{
    PSOutput output;

    float alpha = gTexture.Sample(gSampler, input.texcoord).a;
    if (alpha < 0.5f)
    {
        discard;
    }

    output.id = gObjectID.id;
    return output;
}