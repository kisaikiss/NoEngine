cbuffer VSConst : register(b0)
{
    float4x4 gWorldViewProj;
};

struct VSOutput
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

static const float2 kPos[6] =
{
    { -0.5f, 0.5f },
    { 0.5f, 0.5f },
    { -0.5f, -0.5f },
    { -0.5f, -0.5f },
    { 0.5f, 0.5f },
    { 0.5f, -0.5f }
};
static const float2 kUV[6] = { { 0, 0 }, { 1, 0 }, { 0, 1 }, { 0, 1 }, { 1, 0 }, { 1, 1 } };

VSOutput main(uint id : SV_VertexID)
{
    VSOutput o;
    o.pos = mul(float4(kPos[id], 0.0f, 1.0f), gWorldViewProj);
    o.uv = kUV[id];
    return o;
}