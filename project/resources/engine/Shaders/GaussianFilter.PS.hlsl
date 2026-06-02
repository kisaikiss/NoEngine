
static const float PI = 3.14159265f;

static const float2 kIndex3x3[3][3] =
{
    { { -1.0f, -1.0f }, { 0.0f, -1.0f }, { 1.0f, -1.0f } },
    { { -1.0f, 0.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f } },
    { { -1.0f, 1.0f }, { 0.0f, 1.0f }, { 1.0f, 1.0f } }
};

struct VSOutput
{
    float4 pos : SV_POSITION;
    float2 texcoord : TEXCOORD0;
};

struct PSOutput
{
    float4 color : SV_TARGET0;
};

float gauss(float x, float y, float sigma)
{
    float exponent = -(x * x + y * y) * rcp(2.0f * sigma * sigma);
    float denominator = 2.0f * PI * sigma * sigma;
    return exp(exponent) * rcp(denominator);
}

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

PSOutput main(VSOutput input)
{
    
    uint width, height;
    gTexture.GetDimensions(width, height);
    float2 uvStep = float2(rcp(float(width)), rcp(float(height)));
    
    PSOutput o;
    o.color = 0;
    
    float weight = 0.0f;
    float kernel3x3[3][3];
    for (int x = 0; x < 3; ++x)
    {
        for (int y = 0; y < 3; ++y)
        {
            kernel3x3[x][y] = gauss(kIndex3x3[x][y].x, kIndex3x3[x][y].y, 2.0f);
            weight += kernel3x3[x][y];
            
            float2 texcoord = input.texcoord + kIndex3x3[x][y] * uvStep;
            float3 fetchColor = gTexture.Sample(gSampler, texcoord).rgb;
            o.color.rgb += fetchColor * kernel3x3[x][y];
            
        }
    }
    
    o.color.rgb *= rcp(weight);
    o.color.a = 1.0f;
    return o;
}