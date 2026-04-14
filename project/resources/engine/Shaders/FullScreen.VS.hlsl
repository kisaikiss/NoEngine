
struct VSOutput
{
    float4 pos : SV_POSITION;
    float2 tedcoord : TEXCOORD0;
};

static const int kNumVertex = 3;
static const float4 kPositions[kNumVertex] =
{
    { -1.0f, 1.0f, 0.0f, 1.0f },
    { 3.0f, 1.0f, 0.0f, 1.0f },
    { -1.0f, -3.0f, 0.0f, 1.0f },
};

static const float2 kTexcoords[kNumVertex] =
{
    { 0.0f, 0.0f },
    { 2.0f, 0.0f },
    { 0.0f, 2.0f },
};

VSOutput main(uint vertexId : SV_VertexID)
{
    VSOutput o;
    o.pos = kPositions[vertexId];
    o.tedcoord = kTexcoords[vertexId];
    return o;
}